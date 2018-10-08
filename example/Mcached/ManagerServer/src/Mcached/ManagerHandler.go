package Mcached;

import (
	"encoding/json"
	"net/http"
	"os"
	"os/signal"
	"log"
	"time"
	"context"
	"sync"
	"fmt"
	"go.etcd.io/etcd/clientv3"
	"go.etcd.io/etcd/clientv3/concurrency"
)

const (
	SlotsPrefix						= "/Mcached/Slots/"
	CachedGroupPrefix				= "/Mcached/CachedGroup/"
	GroupIdPrefix					= "/Mcached/GroupId/"		
	ManagerMasterPrefix				= "/Mcached/ManagerMaster/"
)

const (
	HtmlPath						= "../index/"
	IndexFileName					= "../index/index.html"
)

const (
	SlotTotalSize					= "total_size"
	SlotOfGroupid					= "slot_group_id"
	SlotIsAdjust					= "is_move"
	SlotDstGroupid					= "slot_dst_group_id"
)

const (
	ManagerServerTickTimeout		= 200 * time.Millisecond
	EtcdGetRequestTimeout			= 5 * time.Second
	UpdateSlotsCacheGroupDela		= 60 * time.Second
	LeaderSessionLeaseTTL			= 1

	McachedSlotsStart				= 1
	McachedSlotsEnd					= 1025
	CacheGroupIdStart				= 1
)

type Mconf struct {
	LocalAddr string
	EtcdAddr []string
	LogFile string
}

type Mcached interface {
	Run()
}

type McachedContext struct {
	Mutex 				sync.Mutex
	IsCampaign 			bool
	Isleader 			bool
	Leader 				string
}

type McachedManagerServer struct {
	cfg 						Mconf
	quit 						chan os.Signal
	logger 						*log.Logger
	httpserver 					*http.Server
	tick 						*time.Ticker
	cancel 						context.CancelFunc
	ctx 						context.Context

	etcdcli 					*clientv3.Client
	kvcli 						clientv3.KV

	elecCtx 					*McachedContext
	session 					*concurrency.Session
	electc 						chan *concurrency.Election
	election 					*concurrency.Election
	elecerr 					chan error
	elecerrcount 				uint32

	httpredirect 				uint64
	re_mutex 					sync.Mutex

	SlotsGroupidChecked 		bool
	sgc_mutex 					sync.Mutex

	slots_group_mutex			sync.Mutex
	SlotsInfo 					map[uint64]*SlotItem
	SlotsIdlist					[]uint64
	CacheGroupInfo				map[uint64]*CacheGroupItem
	CacheGroupIdList			[]uint64
	updateSlotsCaches			*time.Ticker
}

func (server *McachedManagerServer) Init(cfg *Mconf) bool {
	if cfg == nil {
		log.Fatal("The arg of McachedManagerServer.Init cfg is nil!")
		return false;
	}

	server.cfg = *cfg;
	if cfg.LogFile != "" {
		logFile, err  := os.Create(cfg.LogFile)
		if err != nil {
			log.Fatal("Create logfile failed error:", err)
			return false
		}
		server.logger = log.New(logFile, "", log.Llongfile)
	}

	server.quit = make(chan os.Signal)
	if server.quit == nil {
		log.Fatal("Create Server quit channel failed!")
		return false;
	}
	signal.Notify(server.quit, os.Interrupt)

	server.RegisterHttpHandler()

	server.tick = time.NewTicker(ManagerServerTickTimeout)
	server.ctx, server.cancel = context.WithCancel(context.Background())

	cli, err := clientv3.New(clientv3.Config{
					Endpoints:   cfg.EtcdAddr[:],
					DialTimeout: time.Second * 1,
				})
	if err != nil {
		server.logger.Println("Create etcdclientv3 error:", err)
	}
	server.etcdcli = cli
	server.kvcli = clientv3.NewKV(server.etcdcli)

	server.elecCtx = &McachedContext {
		IsCampaign 	: false,
		Isleader 	: false,
		Leader		: "",
	}

	server.electc = make(chan *concurrency.Election, 2)
	els, err := concurrency.NewSession(server.etcdcli, 
						concurrency.WithTTL(LeaderSessionLeaseTTL),
						concurrency.WithContext(server.ctx))
	if err != nil {
		server.logger.Println("NewSession error:", err)
		return false
	}
	server.session = els
	server.election = concurrency.NewElection(server.session, ManagerMasterPrefix)
	server.elecerr = make(chan error, 2)
	server.elecerrcount = 0

	server.httpredirect = 0

	server.SetSlotsGroupidChecked(false)
	server.SlotsInfo = make(map[uint64]*SlotItem)
	server.SlotsIdlist = make([]uint64, 0)
	server.CacheGroupInfo = make(map[uint64]*CacheGroupItem)
	server.CacheGroupIdList = make([]uint64, 0)
	server.updateSlotsCaches = time.NewTicker(UpdateSlotsCacheGroupDela)
	server.InitSlotsInfo()
	return true;
}

func (server *McachedManagerServer) RegisterHttpHandler() {
	mux := http.NewServeMux()
	sh := &SlotsHandler {
		mserver : server,
	}
	mux.Handle(SlotsPrefix, sh)
	mux.Handle(SlotsPrefix + "/", sh)

	ch := &CachedGroupHandler {
		mserver : server,
	}
	mux.Handle(CachedGroupPrefix, ch)
	mux.Handle(CachedGroupPrefix + "/", ch)

	ih := &IndexHandler {
		mserver : server,
	}
	mux.Handle("/", ih)

	server.httpserver = &http.Server {
		Addr : server.cfg.LocalAddr,
		WriteTimeout : time.Second * 2,
		Handler : mux,
	}
}

func (server *McachedManagerServer) AddRedirectCount() {
	server.re_mutex.Lock()
	defer server.re_mutex.Unlock()
	server.httpredirect++
}

func (server *McachedManagerServer) GetRedirectCount() uint64 {
	server.re_mutex.Lock()
	defer server.re_mutex.Unlock()
	return server.httpredirect
}

func (server *McachedManagerServer) Run() {
	server.logger.Println("McachedManager server running!")
	go func() {
		<-server.quit
		if err := server.httpserver.Close(); err != nil {
			server.logger.Println("Close server:", err)
		}
		server.cancel()
	}()

	go func() {
		for {
			select {
			case <-server.updateSlotsCaches.C:
				server.DoUpdateSlotsCache()
			case <-server.tick.C:
				server.Tick()
			case <-server.electc:
				server.MaybeLeader()
			case <-server.elecerr:
				server.elecerrcount++
			case <-server.ctx.Done():
				return ;
			}
		}
	}()
	
	err := server.httpserver.ListenAndServe()
	if err != nil {
		if err == http.ErrServerClosed {
			server.logger.Println("Server closed under request")
		} else {
			server.logger.Println("Server closed unexpected", err)
		}
	}
	server.logger.Println("Manager Server exited!")
}



func (server *McachedManagerServer) DoUpdateSlotsCache() {
	server.logger.Println("DoUpdateSlotsCache")
	ctx, cancel := context.WithTimeout(context.Background(), 5 * time.Second)
	defer cancel()
	slot_succ := true
	group_succ := true
	// update Slot
	slot_resp, err := server.etcdcli.Get(ctx, SlotsPrefix, clientv3.WithPrefix())
	slot_item_map := make(map[uint64]*SlotItem)
	slot_index_list := make([]uint64, 0)
	if err != nil {
		server.logger.Println("Update slot items failed! error[", err, "]")
		slot_succ = false
	} else {
		for _, KV := range slot_resp.Kvs {
			var item SlotItem
			if err := json.Unmarshal(KV.Value, &item); err != nil {
				server.logger.Println("Unmarshal slot item : ", err)
			}
			slot_item_map[item.Index] = &item
			slot_index_list = append(slot_index_list, item.Index) 
		}
	}

	// update CacheGroup
	group_map := make(map[uint64]*CacheGroupItem)
	group_resp, err := server.etcdcli.Get(ctx, CachedGroupPrefix, clientv3.WithPrefix())
	group_id_list := make([]uint64, 0)
	if err != nil {
		server.logger.Println("Update cache group failed! error[", err, "]")
		group_succ = false
	} else {
		for _, KV := range group_resp.Kvs {
			var item CacheGroupItem
			if err := json.Unmarshal(KV.Value, &item); err != nil {
				server.logger.Println("Unmarshal slot item : ", err)
			}
			group_map[item.GroupId] = &item
			group_id_list = append(group_id_list, item.GroupId)
		}
	}

	server.slots_group_mutex.Lock()
	defer server.slots_group_mutex.Unlock()
	if slot_succ {
		server.logger.Println("DoUpdateSlotsCache update slot:", len(slot_item_map))
		server.SlotsInfo = slot_item_map
		server.SlotsIdlist = slot_index_list
	}

	if group_succ {
		server.logger.Println("DoUpdateSlotsCache update group:", len(group_map))
		server.CacheGroupInfo = group_map
		server.CacheGroupIdList = group_id_list
	}
}

func (server *McachedManagerServer) Tick () {
	lr, err := server.election.Leader(server.ctx)
	if err != nil {
		if err == concurrency.ErrElectionNoLeader {
			server.Campaign()
		} else {
			server.logger.Println("Get leader error:", err)
		}
	} else {
		// maybe start later, the leader is exist! should update campaign!
		server.MaybeUpdateElectCtx(false, 
						string(lr.Kvs[0].Value) == server.cfg.LocalAddr, 
						string(lr.Kvs[0].Value))
	}
}

func (server *McachedManagerServer) MaybeLeader() {
	server.elecCtx.Mutex.Lock()
	defer server.elecCtx.Mutex.Unlock()

	if !server.elecCtx.IsCampaign {
		return
	}

	server.elecCtx.IsCampaign = false
	leader, err := server.election.Leader(server.ctx)
	if err != nil {
		server.logger.Println("Electtion get Leader error:", err)
		return
	}

	if string(leader.Kvs[0].Value) == "" {
		server.logger.Println("Leader msg is empty!")
	}

	server.elecCtx.Isleader = bool(string(leader.Kvs[0].Value) == server.cfg.LocalAddr)
	server.elecCtx.Leader = string(leader.Kvs[0].Value)

	server.logger.Println("Leader:", server.elecCtx.Leader)

	if server.elecCtx.Isleader && !server.HasCheckSlotGroupid() {
		b1, _ := server.MaybeInitSlots()
		b2, _  := server.MaybeUpdateMaxGroupid(0)
		b3, _ := server.MaybeUpdateGroupCount(0)
		if b1 && b2 && b3 {
			server.SetSlotsGroupidChecked(true)
		}
	}
}

func (server *McachedManagerServer) MaybeUpdateElectCtx(iscam, isleader bool, leader string) {
	server.elecCtx.Mutex.Lock()
	defer server.elecCtx.Mutex.Unlock()

	if server.elecCtx.IsCampaign {
		server.logger.Println("IsCampaign:^_^")
		return
	}

	server.elecCtx.IsCampaign = iscam
	server.elecCtx.Isleader = isleader
	server.elecCtx.Leader = leader
}

func (server *McachedManagerServer) Campaign() {
	server.elecCtx.Mutex.Lock()
	defer server.elecCtx.Mutex.Unlock()

	if server.elecCtx.IsCampaign {
		return
	}

	server.elecCtx.IsCampaign = true
	server.elecCtx.Isleader = false
	server.elecCtx.Leader = ""

	go func() {
		ctx, cancel := context.WithTimeout(context.Background(), EtcdGetRequestTimeout)
		defer cancel()
		if err := server.election.Campaign(ctx, server.cfg.LocalAddr); err != nil {
			server.logger.Println("Election Campaign error:", err)
			server.elecerr <- err
		}
		server.electc <- server.election
	}()
}

func (server *McachedManagerServer) IsLeader() (bool, string, error) {
	server.elecCtx.Mutex.Lock()
	defer server.elecCtx.Mutex.Unlock()

	if server.elecCtx.IsCampaign {
		return false, "", concurrency.ErrElectionNoLeader
	}

	return server.elecCtx.Isleader, server.elecCtx.Leader, nil
}

func (server *McachedManagerServer) HasCheckSlotGroupid() bool {
	server.sgc_mutex.Lock()
	defer server.sgc_mutex.Unlock()
	return server.SlotsGroupidChecked
}

func (server *McachedManagerServer) SetSlotsGroupidChecked(val bool) {
	server.sgc_mutex.Lock()
	defer server.sgc_mutex.Unlock()
	server.SlotsGroupidChecked = val
}

func (server *McachedManagerServer) CheckGroupidIsBuild() (bool, error) {
	ctx, cancel := context.WithTimeout(context.Background(), EtcdGetRequestTimeout)
	defer cancel()
	res, err := server.etcdcli.Get(ctx, GroupIdPrefix, clientv3.WithCountOnly(), clientv3.WithPrefix())
	if err != nil {
		server.logger.Println("Checkout GroupId error:", err)
		return false, err
	}
	server.logger.Println("Groupid count:", res.Count)
	return res.Count >= 1, nil
}

func (server *McachedManagerServer) MaybeUpdateMaxGroupid(id int64) (bool, error) {
	init, err := server.CheckGroupidIsBuild()
	if err != nil {
		server.logger.Println("CheckGroupidIsBuild error:", err)
	}
	if init {
		return true, nil
	}
	ctx, cancel := context.WithTimeout(context.Background(), EtcdGetRequestTimeout)
	defer cancel()
	// create MaxGroupid
	val := fmt.Sprintf("%d", id)
	_, err = server.etcdcli.Put(ctx, GroupIdPrefix + "MaxGroupid", val)
	if err != nil {
		server.logger.Println("Create MaxGroupid error:", err)
		return false, err
	}
	return true, nil
}

func (server *McachedManagerServer) MaybeUpdateGroupCount(count int64) (bool, error) {
	init, err := server.CheckGroupidIsBuild()
	if err != nil {
		server.logger.Println("CheckGroupidIsBuild error:", err)
	}
	if init {
		return true, nil
	}
	ctx, cancel := context.WithTimeout(context.Background(), EtcdGetRequestTimeout)
	defer cancel()
	// create MaxGroupid
	val := fmt.Sprintf("%d", count)
	_, err = server.etcdcli.Put(ctx, CachedGroupPrefix + "GroupCount", val)
	if err != nil {
		server.logger.Println("Create MaxGroupid error:", err)
		return false, err
	}
	return true, nil
}

func (server *McachedManagerServer) CheckSlotIsBuild() (bool, error) {
	ctx, cancel := context.WithTimeout(context.Background(), EtcdGetRequestTimeout)
	defer cancel()
	res, err := server.etcdcli.Get(ctx, SlotsPrefix, clientv3.WithCountOnly(), clientv3.WithPrefix())
	if err != nil {
		server.logger.Println("Checkout Slots error:", err)
		return false, err
	}
	server.logger.Println("Slots count:", res.Count)
	return res.Count >= McachedSlotsEnd - McachedSlotsStart, nil
}

func (server *McachedManagerServer) MaybeInitSlots() (bool, error) {
	init, err := server.CheckSlotIsBuild()
	if err != nil {
		server.logger.Println("CheckSlotIsBuild error:", err)
	}
	if init {
		return true, nil
	}
	// createslots
	for i := McachedSlotsStart; i < McachedSlotsEnd; i++ {
		if succ, err := server.CreateSlot(i); succ != true {
			return succ, err
		}
	}
	return true, nil
}

func (server *McachedManagerServer) CreateSlot(index int) (bool, error) {
	if index < 0 {
		server.logger.Println("Create negative slot!")
	}
	slot := &SlotItem {
		Index : uint64(index),
		Groupid : 0,
		IsAdjust: false,
		DstGroupid:0,
	}
	return server.CreateSlotItem(slot)
}

func (server *McachedManagerServer) GetSlotsIndexKey(index uint64) string {
	key_prefix := fmt.Sprintf("%s%d", SlotsPrefix, index)
	return key_prefix
}

func (server *McachedManagerServer) CreateSlotItem (slot *SlotItem) (bool, error) {
	ctx, cancel := context.WithTimeout(context.Background(), EtcdGetRequestTimeout)
	defer cancel()
	if (slot.Index == 0) {
		server.logger.Println("Slot Index is zero!")
	}

	key_prefix := server.GetSlotsIndexKey(slot.Index)
	slot_byte, err := json.Marshal(slot)
	if err != nil {
		server.logger.Println("Json Marshal slot item error:", err)
		return false, err
	}
	
	_, err = server.etcdcli.Put(ctx, key_prefix, string(slot_byte))

	if err != nil {
		server.logger.Println("Create slot item unsucceed!")
		return false, err
	}

	return true, nil
}

func (server *McachedManagerServer) GetCacheGroupIdKey(id uint64) string {
	key_prefix := fmt.Sprintf("%s%d", CachedGroupPrefix, id)
	return key_prefix
}

func (server *McachedManagerServer) CreateCachedGroup (group *CacheGroupItem) (bool, error) {
	ctx, cancel := context.WithTimeout(context.Background(), EtcdGetRequestTimeout)
	defer cancel()
	if (group.GroupId == 0) {
		server.logger.Println("Slot Index is zero!")
	}

	key_prefix := server.GetCacheGroupIdKey(group.GroupId)
	group_byte, err := json.Marshal(group)
	if err != nil {
		server.logger.Println("Json Marshal group item error:", err)
		return false, err
	}
	
	_, err = server.etcdcli.Put(ctx, key_prefix, string(group_byte))

	if err != nil {
		server.logger.Println("Create group item unsucceed!")
		return false, err
	}

	return true, nil
}

func (server *McachedManagerServer) GetSlotsNum() uint64 {
	server.slots_group_mutex.Lock()
	defer server.slots_group_mutex.Unlock()
	return uint64(len(server.SlotsIdlist))
}

func (server *McachedManagerServer) GetNthSlotsIndex(Nth uint64) uint64 {
	server.slots_group_mutex.Lock()
	defer server.slots_group_mutex.Unlock()
	if (Nth < uint64(len(server.SlotsIdlist))) {
		return server.SlotsIdlist[Nth]
	}
	return 0
}

func (server *McachedManagerServer) GetGroupsNum() uint64 {
	server.slots_group_mutex.Lock()
	defer server.slots_group_mutex.Unlock()
	return uint64(len(server.CacheGroupIdList))
}

func (server *McachedManagerServer) GetNthGroupid(Nth uint64) uint64 {
	server.slots_group_mutex.Lock()
	defer server.slots_group_mutex.Unlock()
	if (Nth < uint64(len(server.CacheGroupIdList))) {
		return server.CacheGroupIdList[Nth]
	}
	return 0
}

func (server *McachedManagerServer) InitSlotsInfo() {
	for i := uint64(McachedSlotsStart); i < McachedSlotsEnd; i++ {
		item := &SlotItem {
			Index :	i,
			TotalSize : 0,
			Groupid : 0,
			IsAdjust : false,
			DstGroupid : 0,
		}
		server.SlotsInfo[i] = item
		server.SlotsIdlist = append(server.SlotsIdlist, i)
	}
}

func (server *McachedManagerServer) BuildSlotInfo(slot uint64) SlotInfo {
	ret := SlotInfo {
		SlotItem : *(server.SlotsInfo[slot]),
	}
	cid := server.SlotsInfo[slot].Groupid
	if cid != 0 {
		ret.Cacheaddr = server.CacheGroupInfo[cid].Hosts
	} else {
		ret.Cacheaddr = CacheAddr{
			Master : "",
			Slaves : make([]string, 0),
		}
	}
	return ret
}

func Max(left, right uint64) uint64 {
	if (left >= right) {
		return left
	}
	return right
}

func Min(left, right uint64) uint64 {
	if (left <= right) {
		return left
	}
	return right
}

func (server *McachedManagerServer) GetSlotsInfo(start, size uint64) (* SlotsResponseInfo, error) {
	server.slots_group_mutex.Lock()
	defer server.slots_group_mutex.Unlock()

	start = Max(start, McachedSlotsStart)
	end := Min(start + size, McachedSlotsEnd)

	infos := &SlotsResponseInfo {
		Succeed : false,
		SlotNum : 0,
		SlotTotal : 0,
		Items 	: make([]SlotInfo, 0),
	}

	for i := start; i < end; i++ {
		infos.SlotNum++
		th := server.SlotsIdlist[i]
		infos.Items = append(infos.Items, server.BuildSlotInfo(th))
	}
	infos.SlotTotal = uint64(len(server.SlotsIdlist))
	infos.Succeed = true

	return infos, nil
}

func (server *McachedManagerServer) BuildCacheGroupItem(group uint64) CacheGroupItem {
	if val, ok := server.CacheGroupInfo[group]; ok {
		return *val
	} else {
		server.logger.Panicln("Find key failed!")
	}
	ret := CacheGroupItem {
		GroupId : group,
		Hosts : CacheAddr{
			Master : "",
			Slaves : make([]string, 0),
		},
		SlotsIndex : make([]uint64, 0),
	}
	return ret
}

func (server *McachedManagerServer) GetCacheGroupInfo(start, size uint64) (* CacheGroupResponseInfo, error) {
	server.slots_group_mutex.Lock()
	defer server.slots_group_mutex.Unlock()

	start = Max(start, McachedSlotsStart)
	end := Min(start + size, uint64(len(server.CacheGroupIdList)))

	infos := &CacheGroupResponseInfo {
		Succeed : false,
		GroupNum : 0,
		GroupTotal : 0,
		Items 	: make([]CacheGroupItem, 0),
	}

	for i := start; i < end; i++ {
		infos.GroupNum++
		th := server.CacheGroupIdList[i]
		infos.Items = append(infos.Items, server.BuildCacheGroupItem(th))
	}
	infos.GroupTotal = uint64(len(server.CacheGroupIdList))
	infos.Succeed = true

	return infos, nil
}




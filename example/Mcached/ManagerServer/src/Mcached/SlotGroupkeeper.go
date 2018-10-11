package Mcached

import (
	"log"
	"time"
	"sync"
	"context"
	"errors"
	"net/http"
	"io/ioutil"
	"encoding/json"
	"go.etcd.io/etcd/clientv3"
)

const (
	HttpScheme				= "http"
)

const (
	HtmlPath						= "../index/"
	IndexFileName					= "../index/index.html"
)

const (
	SlotsPrefix						= "/Mcached/Slots/"
	CachedGroupPrefix				= "/Mcached/CachedGroup/"
	GroupIdPrefix					= "/Mcached/GroupId/"		
	ManagerMasterPrefix				= "/Mcached/ManagerMaster/"
)

const (
	UpdateSlotsGroupTickerTimeout		= 10 * time.Second

	McachedSlotsStart				= 1
	McachedSlotsEnd					= 1025
	CacheGroupIdStart				= 1
)

type SlotsRequest struct {
	Version						int16 `json:"version"`
	PageIndex 					uint64 `json:"page_index"`
	PageSize					uint64 `json:"page_size"`
}

type SlotItem struct {
	Index 						uint64 `json:"index"`
	Groupid 					uint64 `json:"group_id"`
	IsAdjust 					bool   `json:"is_adjust"`
	DstGroupid 					uint64 `json:"dst_froup_id"`
}

type CacheAddr struct {
	Master 						string	 `json:"master"`
	Slaves						[]string `json:"slaves"`
}

type SlotInfo struct {
	SlotItem
	Cacheaddr 					CacheAddr `json:"cache_addrs"`
}

type SlotsResponseInfo struct {
	Succeed						bool 		`json:"succeed"`
	SlotNum						uint64 		`json:"slot_num"`
	SlotTotal					uint64		`json:"total_num"`
	Items						[]SlotInfo	`json:"slot_items"`
}

type CacheGroupRequest struct {
	Version						int16 `json:"version"`
	PageIndex 					uint64 `json:"page_index"`
	PageSize					uint64 `json:"page_size"`
}

type CacheGroupItem struct {
	GroupId						uint64	    `json:"group_id"`
	Hosts						CacheAddr   `json:"cached_hosts"`
	SlotsIndex					[]uint64	`json:"slots"`
}

type CacheGroupResponseInfo struct {
	Succeed						bool 		     `json:"succeed"`
	GroupNum					uint64 		     `json:"group_num"`
	GroupTotal					uint64		     `json:"total_num"`
	Items						[]CacheGroupItem `json:"group_items"`
}

type SlotGroupKeeper struct {
	Sgconfig							*SGConfig		

	SlotsGroupMutex						sync.Mutex
	SlotsInfo 							map[uint64]*SlotItem
	SlotsIdList							[]uint64
	CacheGroupInfo						map[uint64]*CacheGroupItem
	CacheGroupIdList					[]uint64
	UpdateSlotsGroupTicker				*time.Ticker

	EtcdClientv3						*clientv3.Client

	HttpServer							*http.Server
	Ctx									*context.Context
	cancel								context.CancelFunc

	Klogger 							*log.Logger
}

type SGConfig struct {
	EtcdEndpoints				[]string
	HttpEndpoints				string
	Klogger 					*log.Logger
}

func NewSlotGroupKeeper(mcontext context.Context, cfg *SGConfig) (*SlotGroupKeeper, error) {
	if cfg == nil {
		return nil, errors.New("SGConfig is nil!")
	}

	ctx, cel := context.WithCancel(mcontext)

	keeper := &SlotGroupKeeper {
		Sgconfig : cfg,
		SlotsInfo : make(map[uint64]*SlotItem),
		SlotsIdList : make([]uint64, 0),
		CacheGroupInfo : make(map[uint64]*CacheGroupItem),
		CacheGroupIdList : make([]uint64, 0),
		UpdateSlotsGroupTicker : time.NewTicker(UpdateSlotsGroupTickerTimeout),

		EtcdClientv3 : nil,
		HttpServer : nil,

		Ctx : &ctx,
		cancel : cel,

		Klogger : cfg.Klogger,
	}
	keeper.InitSlotsInfo()
	keeper.RegisterHttpHandler()

	cv3, err := clientv3.New(clientv3.Config{
		Endpoints:   cfg.EtcdEndpoints,
		DialTimeout: time.Second * 1,
	})
	if err != nil {
		return nil, err
	}
	keeper.EtcdClientv3 = cv3
	return keeper, nil
}

func (keeper *SlotGroupKeeper) Start() {
	go func() {
		for {
			select {
			case <-(*keeper.Ctx).Done():
				keeper.cancel()
			case <-keeper.UpdateSlotsGroupTicker.C:
				keeper.DoUpdateSlotsCache()
			}
		}
	}()

	err := keeper.HttpServer.ListenAndServe()
	if err != nil {
		if err == http.ErrServerClosed {
			keeper.Klogger.Println("Server closed under request")
		} else {
			keeper.Klogger.Println("Server closed unexpected", err)
		}
	}
	keeper.Klogger.Println("SlotGroupKeeper Server exited!")
}

func (keeper *SlotGroupKeeper) RegisterHttpHandler() {
	mux := http.NewServeMux()
	sh := &SlotsListHandler {
		Keeper : keeper,
	}
	mux.Handle(SlotsPrefix, sh)
	mux.Handle(SlotsPrefix + "/", sh)

	ch := &CachedGroupListHandler {
		Keeper : keeper,
	}
	mux.Handle(CachedGroupPrefix, ch)
	mux.Handle(CachedGroupPrefix + "/", ch)

	ih := &IndexHandler {
		Keeper : keeper,
	}
	mux.Handle("/", ih)

	keeper.HttpServer = &http.Server {
		Addr : keeper.Sgconfig.HttpEndpoints,
		WriteTimeout : time.Second * 2,
		Handler : mux,
	}
}

func (keeper *SlotGroupKeeper) DoUpdateSlotsCache() {
	keeper.Klogger.Println("DoUpdateSlotsCache")
	ctx, cancel := context.WithTimeout(context.Background(), 5 * time.Second)
	defer cancel()
	slot_succ := true
	group_succ := true
	// update Slot
	slot_resp, err := keeper.EtcdClientv3.Get(ctx, SlotsPrefix, clientv3.WithPrefix())
	slot_item_map := make(map[uint64]*SlotItem)
	slot_index_list := make([]uint64, 0)
	if err != nil {
		keeper.Klogger.Println("Update slot items failed! error[", err, "]")
		slot_succ = false
	} else {
		for _, KV := range slot_resp.Kvs {
			var item SlotItem
			if err := json.Unmarshal(KV.Value, &item); err != nil {
				keeper.Klogger.Println("Unmarshal slot item : ", err)
			}
			slot_item_map[item.Index] = &item
			slot_index_list = append(slot_index_list, item.Index) 
		}
	}

	// update CacheGroup
	group_map := make(map[uint64]*CacheGroupItem)
	group_resp, err := keeper.EtcdClientv3.Get(ctx, CachedGroupPrefix, clientv3.WithPrefix())
	group_id_list := make([]uint64, 0)
	if err != nil {
		keeper.Klogger.Println("Update cache group failed! error[", err, "]")
		group_succ = false
	} else {
		for _, KV := range group_resp.Kvs {
			var item CacheGroupItem
			if err := json.Unmarshal(KV.Value, &item); err != nil {
				keeper.Klogger.Println("Unmarshal slot item : ", err)
			}
			group_map[item.GroupId] = &item
			group_id_list = append(group_id_list, item.GroupId)
		}
	}

	keeper.SlotsGroupMutex.Lock()
	defer keeper.SlotsGroupMutex.Unlock()
	if slot_succ {
		keeper.Klogger.Println("DoUpdateSlotsCache update slot:", len(slot_item_map))
		keeper.SlotsInfo = slot_item_map
		keeper.SlotsIdList = slot_index_list
	}

	if group_succ {
		keeper.Klogger.Println("DoUpdateSlotsCache update group:", len(group_map))
		keeper.CacheGroupInfo = group_map
		keeper.CacheGroupIdList = group_id_list
	}
}

func (keeper *SlotGroupKeeper) IsLeader() (bool, string, error) {
	return true, "", nil
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

func (keeper *SlotGroupKeeper) InitSlotsInfo() {
	for i := uint64(McachedSlotsStart); i < McachedSlotsEnd; i++ {
		item := &SlotItem {
			Index :	i,
			Groupid : 0,
			IsAdjust : false,
			DstGroupid : 0,
		}
		keeper.SlotsInfo[i] = item
		keeper.SlotsIdList = append(keeper.SlotsIdList, i)
	}
}

func (keeper *SlotGroupKeeper) BuildSlotInfo(slot uint64) SlotInfo {
	ret := SlotInfo {
		SlotItem : *(keeper.SlotsInfo[slot]),
	}
	cid := keeper.SlotsInfo[slot].Groupid
	if cid != 0 {
		ret.Cacheaddr = keeper.CacheGroupInfo[cid].Hosts
	} else {
		ret.Cacheaddr = CacheAddr{
			Master : "",
			Slaves : make([]string, 0),
		}
	}
	return ret
}

func (keeper *SlotGroupKeeper) GetSlotsInfo(start, size uint64) (* SlotsResponseInfo, error) {
	keeper.SlotsGroupMutex.Lock()
	defer keeper.SlotsGroupMutex.Unlock()

	start = Max(start, McachedSlotsStart)
	end := Min(start + size, uint64(len(keeper.SlotsIdList)))

	infos := &SlotsResponseInfo {
		Succeed : false,
		SlotNum : 0,
		SlotTotal : 0,
		Items 	: make([]SlotInfo, 0),
	}

	for i := start; i < end; i++ {
		infos.SlotNum++
		th := keeper.SlotsIdList[i]
		infos.Items = append(infos.Items, keeper.BuildSlotInfo(th))
	}
	infos.SlotTotal = uint64(len(keeper.SlotsIdList))
	infos.Succeed = true

	return infos, nil
}

func (keeper *SlotGroupKeeper) BuildCacheGroupItem(group uint64) CacheGroupItem {
	if val, ok := keeper.CacheGroupInfo[group]; ok {
		return *val
	} else {
		keeper.Klogger.Panicln("Find key failed!")
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

func (keeper *SlotGroupKeeper) GetCacheGroupInfo(start, size uint64) (* CacheGroupResponseInfo, error) {
	keeper.SlotsGroupMutex.Lock()
	defer keeper.SlotsGroupMutex.Unlock()

	start = Max(start, CacheGroupIdStart)
	end := Min(start + size, uint64(len(keeper.CacheGroupIdList)))

	infos := &CacheGroupResponseInfo {
		Succeed : false,
		GroupNum : 0,
		GroupTotal : 0,
		Items 	: make([]CacheGroupItem, 0),
	}

	for i := start; i < end; i++ {
		infos.GroupNum++
		th := keeper.CacheGroupIdList[i]
		infos.Items = append(infos.Items, keeper.BuildCacheGroupItem(th))
	}
	infos.GroupTotal = uint64(len(keeper.CacheGroupIdList))
	infos.Succeed = true

	return infos, nil
}

type IndexHandler struct {
	Keeper *SlotGroupKeeper
}

func (ser *IndexHandler) ServeHTTP(response http.ResponseWriter, request *http.Request) {
	http.ServeFile(response, request, IndexFileName)
}

type SlotsListHandler struct {
	Keeper *SlotGroupKeeper
}

func (ser *SlotsListHandler) ServeHTTP(response http.ResponseWriter, request *http.Request) {
	emptyres := &SlotsResponseInfo {
		Succeed : false,
		SlotNum : 0,
		SlotTotal : 0,
		Items   : make([]SlotInfo, 0),
	}

	sq := &SlotsRequest {}
	body, _ := ioutil.ReadAll(request.Body)
	if err := json.Unmarshal(body, sq); err != nil {
		response.WriteHeader(500)
		if error_res, err := json.Marshal(emptyres); err == nil {
			response.Write(error_res)
		}
		return
	}

	ser.Keeper.Klogger.Println("PageIndex:", sq.PageIndex, " PageSize:", sq.PageSize)

	if (sq.PageIndex < 1) {
		sq.PageIndex = 1
	}
	start_index := sq.PageSize * (sq.PageIndex - 1) + 1

	slotsinfo, err := ser.Keeper.GetSlotsInfo(start_index, sq.PageSize)
	slotsinfo.Succeed = true
	httpres, err := json.Marshal(slotsinfo)
	if err != nil {
		response.WriteHeader(500)
		if error_res, err := json.Marshal(emptyres); err == nil {
			response.Write(error_res)
		}
		return
	}
	response.Write(httpres)
}

type CachedGroupListHandler struct {
	Keeper *SlotGroupKeeper
}

func (ser *CachedGroupListHandler) ServeHTTP(response http.ResponseWriter, request *http.Request) {	
	emptyres := &CacheGroupResponseInfo {
		Succeed : false,
		GroupNum : 0,
		GroupTotal : 0,
		Items   : make([]CacheGroupItem, 0),
	}

	sq := &CacheGroupRequest {}
	body, _ := ioutil.ReadAll(request.Body)
	if err := json.Unmarshal(body, sq); err != nil {
		response.WriteHeader(500)
		if error_res, err := json.Marshal(emptyres); err == nil {
			response.Write(error_res)
		}
		return
	}
	if (sq.PageIndex < 1) {
		sq.PageIndex = 1
	}
	start_index := sq.PageSize * (sq.PageIndex - 1) + 1

	groupsinfo, err := ser.Keeper.GetCacheGroupInfo(start_index, sq.PageSize)
	groupsinfo.Succeed = true
	httpres, err := json.Marshal(groupsinfo)
	if err != nil {
		response.WriteHeader(500)
		if error_res, err := json.Marshal(emptyres); err == nil {
			response.Write(error_res)
		}
		return
	}
	response.Write(httpres)
}

func DeleteSlotFromGroup(slot *SlotItem, group *CacheGroupItem) (bool, error) {
	if slot.Groupid != group.GroupId {
		return false, errors.New("The slot not belongs to this group")
	} 
	
	index := -1

	for i := 0; i < len(group.SlotsIndex); i++ {
		if group.SlotsIndex[i] == slot.Index {
			index = i
			break
		}
	}

	if index == -1 {
		return false, errors.New("Slot can't found in this Group!")
	}

	tmp := make([]uint64, 0)
	tmp = append(tmp, group.SlotsIndex[:index]...)
	tmp = append(tmp, group.SlotsIndex[index + 1:]...)

	group.SlotsIndex = tmp
	slot.Groupid = 0
	slot.IsAdjust = false
	slot.DstGroupid = 0

	return true, nil
}

func AddSlotToGroup(slot *SlotItem, group *CacheGroupItem) (bool, error) {
	if slot.Groupid == group.GroupId {
		return false, errors.New("The slot belongs to this group")
	} 
	
	index := -1

	for i := 0; i < len(group.SlotsIndex); i++ {
		if group.SlotsIndex[i] == slot.Index {
			index = i
			break
		}
	}

	if index != -1 {
		return false, errors.New("Slot found in this Group!")
	}

	group.SlotsIndex = append(group.SlotsIndex, slot.Index)
	slot.Groupid = group.GroupId
	slot.IsAdjust = false
	slot.DstGroupid = 0

	return true, nil
}

func SlotInGroup(slot *SlotItem, group *CacheGroupItem) (bool, error) {
	if slot.Groupid != group.GroupId {
		return false, errors.New("The slot belongs to this group")
	} 
	
	index := -1

	for i := 0; i < len(group.SlotsIndex); i++ {
		if group.SlotsIndex[i] == slot.Index {
			index = i
			break
		}
	}

	if index == -1 {
		return false, errors.New("Slot not found in this Group!")
	}

	return true, nil
}
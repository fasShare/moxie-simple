package Mcached

import (
	"net/http"
	//"io/ioutil"
	"encoding/json"
	"go.etcd.io/etcd/clientv3/concurrency"
)

const (
	HttpScheme				= "http"
)

type SlotsRequest struct {
	Version						int16 `json:"version"`
	StartIndex 					uint64 `json:"slot_start"`
	EndIndex					uint64 `json:"slot_end"`
}

type SlotItem struct {
	Index 						uint64 `json:"index"`
	TotalSize 					uint64 `json:"total_size"`
	Groupid 					uint64 `json:"group_id"`
	IsAdjust 					bool   `json:"is_adjust"`
	DstGroupid 					uint64 `json:"dst_froup_id"`
}

type CacheGroupItem struct {
	GroupId						uint64	    `json:"group_id"`
	Hosts						CacheAddr   `json:"cached_hosts"`
	SlotsIndex					[]uint64	`json:"slots"`
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
	SlotNum						int64 		`json:"slot_num"`
	Items						[]SlotInfo	`json:"slot_items"`
}

func GetRedirectRequestUrl(request *http.Request, redirect_host string) string {
	if request == nil {
		return ""
	}
	return redirect_host + request.RequestURI
}

type IndexHandler struct {
	mserver *McachedManagerServer
}

func (ser *IndexHandler) ServeHTTP(response http.ResponseWriter, request *http.Request) {
	isleader, leader, err := ser.mserver.IsLeader()
	if err != nil{
		if err == concurrency.ErrElectionNoLeader {
			// Service Unavailable
			ser.mserver.ErrorLogger("Service Unavailable")
			response.WriteHeader(503)
		} else {
			// Internal Server Error
			ser.mserver.ErrorLogger("Internal Server Error")
			response.WriteHeader(500)
		}
		return
	}
	
	if isleader == false {
		if leader == "" {
			ser.mserver.ErrorLogger("Internal Server Error leader status!")
			response.WriteHeader(500)
		} else {
			ser.mserver.AddRedirectCount()
			ser.mserver.ErrorLogger("Request was redireted!")
			http.Redirect(response, request, GetRedirectRequestUrl(request, leader), 307)
		}
		return
	}
	http.ServeFile(response, request, IndexFileName)
}

type SlotsHandler struct {
	mserver *McachedManagerServer
}

func (ser *SlotsHandler) ServeHTTP(response http.ResponseWriter, request *http.Request) {
	isleader, leader, err := ser.mserver.IsLeader()
	if err != nil{
		if err == concurrency.ErrElectionNoLeader {
			// Service Unavailable
			ser.mserver.ErrorLogger("Service Unavailable")
			response.WriteHeader(503)
		} else {
			// Internal Server Error
			ser.mserver.ErrorLogger("Internal Server Error")
			response.WriteHeader(500)
		}
		return
	}
	
	if isleader == false {
		if leader == "" {
			ser.mserver.ErrorLogger("Internal Server Error leader status!")
			response.WriteHeader(500)
		} else {
			ser.mserver.AddRedirectCount()
			ser.mserver.ErrorLogger("Request was redireted!")
			http.Redirect(response, request, GetRedirectRequestUrl(request, leader), 307)
		}
		return
	}
/*
	sq := &SlotsRequest {}
	body, _ := ioutil.ReadAll(request.Body)
	json.Unmarshal(body, sq)
	slotsinfo, err := ser.mserver.GetSlotsInfo(sq.StartIndex, sq.EndIndex)
*/
	slotsinfo, err := ser.mserver.GetSlotsInfo(0, 10)
	res, err := json.Marshal(slotsinfo)
	if err != nil {
		response.WriteHeader(500)
		return
	}
	response.Write(res)
}

type CachedGroupHandler struct {
	mserver *McachedManagerServer
}

func (ser *CachedGroupHandler) ServeHTTP(response http.ResponseWriter, request *http.Request) {
	isleader, leader, err := ser.mserver.IsLeader()
	if err != nil{
		if err == concurrency.ErrElectionNoLeader {
			// Service Unavailable
			ser.mserver.ErrorLogger("Service Unavailable")
			response.WriteHeader(503)
		} else {
			// Internal Server Error
			ser.mserver.ErrorLogger("Internal Server Error")
			response.WriteHeader(500)
		}
		return
	}
	
	if isleader == false {
		if leader == "" {
			ser.mserver.ErrorLogger("Internal Server Error leader status!")
			response.WriteHeader(500)
		} else {
			ser.mserver.AddRedirectCount()
			ser.mserver.ErrorLogger("Request was redireted!")
			http.Redirect(response, request, GetRedirectRequestUrl(request, leader), 307)
		}
		return
	}
	response.Write([]byte(GetRedirectRequestUrl(request, leader)))
}
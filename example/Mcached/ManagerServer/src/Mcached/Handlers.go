package Mcached

import (
	"net/http"
	"io/ioutil"
	"encoding/json"
	"go.etcd.io/etcd/clientv3/concurrency"
)

const (
	HttpScheme				= "http"
)

type SlotsRequest struct {
	Version						int16 `json:"version"`
	PageIndex 					uint64 `json:"page_index"`
	PageSize					uint64 `json:"page_size"`
}

type SlotItem struct {
	Index 						uint64 `json:"index"`
	TotalSize 					uint64 `json:"total_size"`
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
			ser.mserver.logger.Println("Service Unavailable")
			response.WriteHeader(503)
		} else {
			// Internal Server Error
			ser.mserver.logger.Println("Internal Server Error")
			response.WriteHeader(500)
		}
		return
	}
	
	if isleader == false {
		if leader == "" {
			ser.mserver.logger.Println("Internal Server Error leader status!")
			response.WriteHeader(500)
		} else {
			ser.mserver.AddRedirectCount()
			ser.mserver.logger.Println("Request was redireted!")
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
			ser.mserver.logger.Println("Service Unavailable")
			response.WriteHeader(503)
		} else {
			// Internal Server Error
			ser.mserver.logger.Println("Internal Server Error")
			response.WriteHeader(500)
		}
		return
	}
	
	if isleader == false {
		if leader == "" {
			ser.mserver.logger.Println("Internal Server Error leader status!")
			response.WriteHeader(500)
		} else {
			ser.mserver.AddRedirectCount()
			ser.mserver.logger.Println("Request was redireted!")
			http.Redirect(response, request, GetRedirectRequestUrl(request, leader), 307)
		}
		return
	}

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

	ser.mserver.logger.Println("PageIndex:", sq.PageIndex, " PageSize:", sq.PageSize)

	if (sq.PageIndex < 1) {
		sq.PageIndex = 1
	}
	start_index := sq.PageSize * (sq.PageIndex - 1) + 1

	slotsinfo, err := ser.mserver.GetSlotsInfo(start_index, sq.PageSize)
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

type CachedGroupHandler struct {
	mserver *McachedManagerServer
}

func (ser *CachedGroupHandler) ServeHTTP(response http.ResponseWriter, request *http.Request) {
	isleader, leader, err := ser.mserver.IsLeader()
	if err != nil{
		if err == concurrency.ErrElectionNoLeader {
			// Service Unavailable
			ser.mserver.logger.Println("Service Unavailable")
			response.WriteHeader(503)
		} else {
			// Internal Server Error
			ser.mserver.logger.Println("Internal Server Error")
			response.WriteHeader(500)
		}
		return
	}
	
	if isleader == false {
		if leader == "" {
			ser.mserver.logger.Println("Internal Server Error leader status!")
			response.WriteHeader(500)
		} else {
			ser.mserver.AddRedirectCount()
			ser.mserver.logger.Println("Request was redireted!")
			http.Redirect(response, request, GetRedirectRequestUrl(request, leader), 307)
		}
		return
	}
	

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

	groupsinfo, err := ser.mserver.GetCacheGroupInfo(start_index, sq.PageSize)
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
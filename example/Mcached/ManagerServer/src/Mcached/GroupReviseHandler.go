package Mcached

import (
	"encoding/json"
	"io/ioutil"
	"net/http"
	"go.etcd.io/etcd/clientv3/concurrency"
)

const (
	CmdCreateCacheGroup 		= 	0
	CmdDeleteCacheGroup			= 	1
	CmdAddSlot					= 	2
	CmdMoveSlot					= 	3
	CmdDelSlot					= 	4
	CmdSlotStartMove			= 	5
)

type GroupReviseHandler struct {
	server *ManagerServer
}

type GroupReviseRequest struct {
	Cmdtype					uint32	`json:"cmd_type"`
	SourceGroupId			uint64	`json:"source_id"`
	DestGroupId				uint64	`json:"dest_id"`
	SlotId 					uint64	`json:"slot_id"`
}

type GroupReviseResponse struct {
	Succeed					bool	`json:"succeed"`
	Msg 					string	`json:"msg"`
}

func GetRedirectRequestUrl(request *http.Request, redirect_host string) string {
	if request == nil {
		return ""
	}
	return redirect_host + request.RequestURI
}

func (handler *GroupReviseHandler) ServeHTTP(response http.ResponseWriter, request *http.Request) {
	isleader, leader, err := handler.server.IsLeader()
	if err != nil{
		if err == concurrency.ErrElectionNoLeader {
			// Service Unavailable
			handler.server.logger.Println("Service Unavailable")
			response.WriteHeader(503)
		} else {
			// Internal Server Error
			handler.server.logger.Println("Internal Server Error")
			response.WriteHeader(500)
		}
		return
	}
	
	if isleader == false {
		if leader == "" {
			handler.server.logger.Println("Internal Server Error leader status!")
			response.WriteHeader(500)
		} else {
			handler.server.logger.Println("Request was redireted!")
			http.Redirect(response, request, GetRedirectRequestUrl(request, leader), 307)
		}
		return
	}

	Httpres := &GroupReviseResponse {
		Succeed : false,
		Msg : "",
	}

	grr := &GroupReviseRequest {}
	body, _ := ioutil.ReadAll(request.Body)
	if err := json.Unmarshal(body, grr); err != nil {
		response.WriteHeader(500)
		handler.server.logger.Println("json.Unmarshal Request failed:", err)
		if error_res, err := json.Marshal(Httpres); err == nil {
			response.Write(error_res)
		}
		return
	}

	switch grr.Cmdtype {
	case CmdCreateCacheGroup:
		handler.HandleCreateCacheGroup(grr, Httpres)
	case CmdDeleteCacheGroup:
		handler.HandleDeleteCacheGroup(grr, Httpres)
	case CmdAddSlot:
		handler.HandleAddSlot(grr, Httpres)
	case CmdDelSlot:
		handler.HandleDelSlot(grr, Httpres)
	case CmdMoveSlot:
		handler.HandleMoveSlot(grr, Httpres)
	case CmdSlotStartMove:
		handler.HandleSlotStartMove(grr, Httpres)
	default:
		Httpres.Msg = "Cmd not found!"
		Httpres.Succeed = false
	}

	if error_res, err := json.Marshal(Httpres); err == nil {
		response.Write(error_res)
	} else {
		response.WriteHeader(500)
		handler.server.logger.Println("Marshal failed!")
	}
}

func (handler *GroupReviseHandler) HandleCreateCacheGroup(request *GroupReviseRequest, response *GroupReviseResponse) {
	Item := &CacheGroupItem {
		GroupId	: 0,
		Hosts	: CacheAddr {
			Master : "",
			Slaves : make([]string, 0),
		},
		SlotsIndex : make([]uint64, 0),
	}

	if succ, err := handler.server.CreateCachedGroup(Item); err == nil {
		response.Succeed = succ
		response.Msg = "ok"
	} else {
		response.Succeed = succ
		response.Msg = err.Error()
	}
}

func (handler *GroupReviseHandler) HandleDeleteCacheGroup(request *GroupReviseRequest, response *GroupReviseResponse) {
	if succ, err := handler.server.DeleteCachedGroup(request.SourceGroupId); err == nil {
		response.Succeed = succ
		response.Msg = "ok"
	} else {
		response.Succeed = succ
		response.Msg = err.Error()
	}
}

func (handler *GroupReviseHandler) HandleDelSlot(request *GroupReviseRequest, response *GroupReviseResponse) {
	if succ, err := handler.server.DeleteSlotfromGroup(request.SlotId, request.SourceGroupId); err == nil {
		response.Succeed = succ
		response.Msg = "ok"
	} else {
		response.Succeed = succ
		response.Msg = err.Error()
	}
}

func (handler *GroupReviseHandler) HandleAddSlot(request *GroupReviseRequest, response *GroupReviseResponse) {
	if succ, err := handler.server.AddSlotToGroup(request.SlotId, request.SourceGroupId); err == nil {
		response.Succeed = succ
		response.Msg = "ok"
	} else {
		response.Succeed = succ
		response.Msg = err.Error()
	}
}

func (handler *GroupReviseHandler) HandleMoveSlot(request *GroupReviseRequest, response *GroupReviseResponse) {
	if succ, err := handler.server.MoveSlotToGroup(request.SlotId, request.SourceGroupId, request.DestGroupId); err == nil {
		response.Succeed = succ
		response.Msg = "ok"
	} else {
		response.Succeed = succ
		response.Msg = err.Error()
	}
}

func (handler *GroupReviseHandler) HandleSlotStartMove(request *GroupReviseRequest, response *GroupReviseResponse) {
	if succ, err := handler.server.StartMoveSlotToGroup(request.SlotId, request.SourceGroupId, request.DestGroupId); err == nil {
		response.Succeed = succ
		response.Msg = "ok"
	} else {
		response.Succeed = succ
		response.Msg = err.Error()
	}
}
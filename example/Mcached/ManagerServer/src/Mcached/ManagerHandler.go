package Mcached;

import (
	"net/http"
	"os"
	"os/signal"
	"log"
	"time"
)

const (
	SlotsPrefix						= "/McachedSlots"
	CachedGroupPrefix				= "/McachedCachedGroup"
)

type Mconf struct {
	LocalAddr string
	EtcdAddr []string
	LogFile string
}

type Mcached interface {
	Run()
}

type McachedManagerServer struct {
	cfg Mconf
	quit chan os.Signal
	logger *log.Logger
	httpserver *http.Server
}

func (server *McachedManagerServer) InfoLogger(info ...interface{}) {
	if server.logger == nil {
		return
	}
	server.logger.SetPrefix("[INFO]")
	server.logger.Println(info)
}

func (server *McachedManagerServer) WarnLogger(info ...interface{}) {
	if server.logger == nil {
		log.Panicln("server.logger is nil!")
		return
	}
	server.logger.SetPrefix("[WARN]")
	server.logger.Println(info)
}

func (server *McachedManagerServer) ErrorLogger(info ...interface{}) {
	if server.logger == nil {
		log.Panicln("server.logger is nil!")
		return
	}
	server.logger.SetPrefix("[ERROR]")
	server.logger.Println(info)
}

func (server *McachedManagerServer) FatalLogger(info ...interface{}) {
	if server.logger == nil {
		log.Panicln("server.logger is nil!")
		return
	}
	server.logger.SetPrefix("[FATAL]")
	server.logger.Panicln(info)
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

	return true;
}

func (server *McachedManagerServer) Run() {
	server.InfoLogger("McachedManager server running!")
	go func() {
		<-server.quit
		if err := server.httpserver.Close(); err != nil {
			server.FatalLogger("Close server:", err)
		}
	}()

	err := server.httpserver.ListenAndServe()
	if err != nil {
		if err == http.ErrServerClosed {
			server.ErrorLogger("Server closed under request")
		} else {
			server.ErrorLogger("Server closed unexpected", err)
		}
	}
	server.InfoLogger("Manager Server exited!")
}

type IndexHandler struct {
	mserver *McachedManagerServer
}

func (ser *IndexHandler) ServeHTTP(response http.ResponseWriter, request *http.Request) {
	response.Write([]byte("IndexHandler hello!"))
}

type SlotsHandler struct {
	mserver *McachedManagerServer
}

func (ser *SlotsHandler) ServeHTTP(response http.ResponseWriter, request *http.Request) {
	response.Write([]byte("SlotsHandler hello!"))
}

type CachedGroupHandler struct {
	mserver *McachedManagerServer
}

func (ser *CachedGroupHandler) ServeHTTP(response http.ResponseWriter, request *http.Request) {
	response.Write([]byte("CachedGroupHandler hello!"))
}
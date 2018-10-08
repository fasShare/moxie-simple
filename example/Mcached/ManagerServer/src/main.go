package main;

import (
	"Mcached"
	"log"
	"os"
	"strings"
)

func main() {
	cfg := &Mcached.Mconf {
		LocalAddr : "127.0.0.1:8899",
		EtcdAddr : make([]string, 0, 0),
		LogFile : "",
	}
	// -listen-client-urls 'http://localhost:32379'
	// -listen-mcached-url 'http://localhost:8866'
	// -mcached-log-file log_file_name
	for i := 0; i < len(os.Args); i++ {
		if os.Args[i] == "--advertise-client-urls" {
			if i + 1 < len(os.Args) {
				url := os.Args[i + 1]
				urls := strings.Split(url, ",")
				for j := 0; j < len(urls); j++ { // http://
					cfg.EtcdAddr = append(cfg.EtcdAddr, urls[j][7:])
				}
			} else {
				log.Fatal("The listen-client-urls value format is incorrect!")
			}
		}

		if os.Args[i] == "--listen-mcached-url" {
			if i + 1 < len(os.Args) {
				cfg.LocalAddr = os.Args[i + 1][7:]
			} else {
				log.Fatal("The listen-mcached-url value format is incorrect!")
			}
		}

		if os.Args[i] == "--mcached-log-file" {
			if i + 1 < len(os.Args) {
				cfg.LogFile = os.Args[i + 1]
			} else {
				log.Fatal("The listen-mcached-url value format is incorrect!")
			}
		}
	}

	log.Println("Log_file:", cfg.LogFile, " Local_addr:", cfg.LocalAddr, " Etcd_addr:", cfg.EtcdAddr)
	var server Mcached.McachedManagerServer
	server.Init(cfg)
	server.Run()
}
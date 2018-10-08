package main;

import (
	"time"
	"Mcached"
	"log"
	"os"
	"strings"
	"go.etcd.io/etcd/clientv3"
	"context"
	"fmt"
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


	cli, err := clientv3.New(clientv3.Config{
		Endpoints:   cfg.EtcdAddr[:],
		DialTimeout: time.Second * 1,
	})
	if err != nil {
		log.Panicln("Create etcdclientv3 error:", err)
	}

	ctx, cancel := context.WithTimeout(context.Background(), 5 * time.Second)
	resp, err := cli.Get(ctx, "/Mcached/Slots/", clientv3.WithPrefix())
	cancel()
	if err != nil {
		log.Fatal(err)
	}
	fmt.Println("Slots count:", resp.Count)
	for _, ev := range resp.Kvs {
		fmt.Printf("%s : %s\n", ev.Key, ev.Value)
	}	

	// "/Mcached/GroupId/"
	ctx, cancel = context.WithTimeout(context.Background(), 5 * time.Second)
	resp, err = cli.Get(ctx, "/Mcached/GroupId/", clientv3.WithPrefix())
	cancel()
	if err != nil {
		log.Fatal(err)
	}
	fmt.Println("GroupId count:", resp.Count)
	for _, ev := range resp.Kvs {
		fmt.Printf("%s : %s\n", ev.Key, ev.Value)
	}

	var server Mcached.McachedManagerServer
	server.Init(cfg)
	server.Run()
}
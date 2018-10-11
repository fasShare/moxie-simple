package main

import (
	"flag"
	"fmt"
	"github.com/bradfitz/gomemcache/memcache"
	"strconv"
	"sync"
	"time"
)

func GetStringInSize(size uint, char string) (string, error) {
	ret := char
	for uint(len(ret)) < size {
		ret += ret
	}
	return ret[:size], nil
}

func main() {
	routinenum := flag.Int("rn", 4, "Go routine Num!")
	addr := flag.String("ip", "127.0.0.1:11211", "The address of redis server!")
	keylen := flag.Uint("keylen", 30, "The default length of key!")
	datalen := flag.Uint("datalen", 128, "The default length of data!")
	reqs := flag.Int("reqs", 10000, "The times of this test!")
	flag.Parse()

	if *routinenum <= 0 || *reqs <= 0 {
		fmt.Println("Args Error!")
		return
	}

	fmt.Printf("rn = %d, addr = %s, keylen = %d, datalen = %d, reqs = %d\n", *routinenum, *addr, *keylen, *datalen, *reqs)

	var wg sync.WaitGroup
	wg.Add(*routinenum)

	var print_mutex sync.Mutex
	var total_reqs uint64 = 0
	var total_misshits uint64 = 0
	start_time := time.Now()
	for i := 0; i < *routinenum; i++ {
		go func(id int, addr string, kenlen, datalen uint, reqs int) {
			memcachedclient := memcache.New(addr)
			if memcachedclient == nil {
				wg.Add(-1)
				return
			}
			defer wg.Add(-1)
			// do request
			valueprefix, _ := GetStringInSize(datalen, "D")
			keyprefix, _ := GetStringInSize(kenlen, "K")
			var misshits uint64 = 0
			var rq int
			for rq = 0; rq < reqs; rq++ {
				key := strconv.Itoa(i) + keyprefix + strconv.Itoa(rq)
				value := strconv.Itoa(i) + valueprefix + strconv.Itoa(rq)
				err := memcachedclient.Set(&memcache.Item{Key: key, Value: []byte(value)})
				if err != nil {
					break
				}

				reply, err := memcachedclient.Get(key)
				if err != nil {
					fmt.Println(reply)
					break
				}

				if string(reply.Value[:]) != value {
					misshits++
				}
			}
			print_mutex.Lock()
			total_reqs += uint64(rq)
			total_misshits += uint64(misshits)
			fmt.Printf("In goroutine:%d, reqs=%d, misshits=%d\n", id, rq, misshits)
			print_mutex.Unlock()
		}(i, *addr, *keylen, *datalen, *reqs)
	}

	wg.Wait()
	end_time := time.Now()
	total_time := (end_time.UnixNano() - start_time.UnixNano()) / int64(time.Millisecond)
	persec := float64(total_reqs) / float64(total_time) * 1000
	fmt.Printf("total_time:[%d]ms total_reqs:[%d] misshits:[%d] persec:[%f]\n", total_time, total_reqs, total_misshits, persec)
}

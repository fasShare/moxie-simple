package main

import (
	"flag"
	"fmt"
	"github.com/gomodule/redigo/redis"
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
	addr := flag.String("ip", "127.0.0.1:6379", "The address of redis server!")
	keylen := flag.Uint("keylen", 30, "The default length of key!")
	datalen := flag.Uint("datalen", 128, "The default length of data!")
	reqs := flag.Int("reqs", 10000, "The times of this test!")
	flag.Parse()

	if *routinenum <= 0 || *reqs <= 0 {
		fmt.Println("Args Error!")
		return
	}

	fmt.Printf("rn = %d, ip = %s, keylen = %d, datalen = %d, reqs = %d\n", *routinenum, *addr, *keylen, *datalen, *reqs)

	var wg sync.WaitGroup
	wg.Add(*routinenum)

	var print_mutex sync.Mutex
	var total_reqs uint64 = 0
	var total_misshits uint64 = 0
	start_time := time.Now()
	for i := 0; i < *routinenum; i++ {
		go func(id int, addr string, kenlen, datalen uint, reqs int) {
			redisclient, err := redis.Dial("tcp", addr)
			if err != nil {
				wg.Add(-1)
				return
			}
			defer redisclient.Close()
			defer wg.Add(-1)
			// do request
			valueprefix, _ := GetStringInSize(datalen, "D")
			keyprefix, _ := GetStringInSize(kenlen, "K")
			var misshits uint64 = 0
			var rq int
			for rq = 0; rq < reqs; rq++ {
				key := strconv.Itoa(i) + keyprefix + strconv.Itoa(rq)
				value := strconv.Itoa(i) + valueprefix + strconv.Itoa(rq)
				reply, err := redisclient.Do("set", key, value)
				if err != nil {
					fmt.Println(reply)
					break
				}

				reply, err = redisclient.Do("get", key)
				if err != nil {
					fmt.Println(reply)
					break
				}
				ret_value := reply.([]byte)
				if string(ret_value[:]) != value {
					misshits++
				}
			}
			print_mutex.Lock()
			total_reqs += uint64(rq)
			total_misshits += uint64(misshits)
			if misshits != 0 {
				fmt.Printf("In goroutine:%d, reqs=%d, misshits=%d\n", id, rq, misshits)
			}
			print_mutex.Unlock()
		}(i, *addr, *keylen, *datalen, *reqs)
	}

	wg.Wait()
	end_time := time.Now()
	total_time := (end_time.UnixNano() - start_time.UnixNano()) / int64(time.Millisecond)
	persec := float64(total_reqs) / float64(total_time) * 1000
	fmt.Printf("total_time:[%d]ms total_reqs:[%d] misshits:[%d] persec:[%f]\n", total_time, total_reqs, total_misshits, persec)
}

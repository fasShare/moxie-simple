#!/bin/sh

./Mcached "127.0.0.1:8901,127.0.0.1:8902,127.0.0.1:8903" "127.0.0.1" 8901 "./data1/" 6379 &
./Mcached "127.0.0.1:8901,127.0.0.1:8902,127.0.0.1:8903" "127.0.0.1" 8902 "./data2/" 6479 &
./Mcached "127.0.0.1:8901,127.0.0.1:8902,127.0.0.1:8903" "127.0.0.1" 8903 "./data3/" 6579 &


#!/bin/bash
cur_dir=`pwd`
#export mypath=$(cd ${cur_dir}; cd ../; pwd)
mypath=`cd ${cur_dir}; cd ../; pwd`
GOPATH=$GOPATH:${mypath}
go build -o McachedManager
mkdir -p bin
mv McachedManager ./bin/

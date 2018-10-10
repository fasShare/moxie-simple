#!/bin/bash

curl -H "Content-Type:application/json" -X POST --data '{"version":1, "page_index":0, "page_size":1024}' http://127.0.0.1:8868/Mcached/CachedGroup/
echo ""

#!/bin/bash

curl -H "Content-Type:application/json" -X POST --data '{"version":1, "page_index":20, "page_size":20}' http://127.0.0.1:8868/Mcached/Slots/


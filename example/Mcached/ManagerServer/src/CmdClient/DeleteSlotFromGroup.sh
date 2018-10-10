#!/bin/bash

if [ $# -ne "2" ]; then
    echo "Usage:DeleteSlotFromGroup.sh slotid groupid"
    exit -1
fi

curl -H "Content-Type:application/json" -X POST --data '{"cmd_type":4, "source_id":'${2}', "dest_id":0, "slot_id":'${1}'}' http://127.0.0.1:8898/Mcached/GroupRevise/
echo ""
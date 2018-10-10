#!/bin/bash

if [ $# -ne "3" ]; then
    echo "Usage:MoveSlotToGroup.sh slotid srcgroupid destgroupid"
    exit -1
fi

curl -H "Content-Type:application/json" -X POST --data '{"cmd_type":3, "source_id":'${2}', "dest_id":'${3}', "slot_id":'${1}'}' http://127.0.0.1:8898/Mcached/GroupRevise/
echo ""
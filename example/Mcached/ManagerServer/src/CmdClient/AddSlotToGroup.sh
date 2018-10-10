#!/bin/bash

if [ $# -ne 2 ]; then
    echo "Usage:AddSlotToGroup.sh slotid Groupid"
    exit 0
fi

curl -H "Content-Type:application/json" -X POST --data '{"cmd_type":2, "source_id":'${2}', "dest_id":0, "slot_id":'${1}'}' http://127.0.0.1:8898/Mcached/GroupRevise/
echo ""
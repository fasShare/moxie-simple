#!/bin/bash
curl -H "Content-Type:application/json" -X POST --data '{"cmd_type":0, "source_id":0, "dest_id":0, "slot_id":0}' http://127.0.0.1:8898/Mcached/GroupRevise/
echo ""

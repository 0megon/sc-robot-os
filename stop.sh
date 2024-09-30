#!/bin/bash

python3 web-interface/web-interface.py &
killall ./sensor/sensor-handle
killall ./qr-cam/qr-cam
killall ./cs/control-sys
./purge-database.sh
echo > cs/program-calls.txt
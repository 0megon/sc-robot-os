#!/bin/bash

# Reset database
./purge-database.sh
python3 ./web-interface/web-interface.py &
./sensor/sensor-handle &
./qr-cam/qr-cam &
./cs/control-sys &

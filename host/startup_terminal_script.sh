#!/bin/bash

sudo chmod 777 /dev/ttyUSB*
sudo chmod 777 /dev/video*

sleep 1

cd /home/car/GCXL2024/host/build/

./host

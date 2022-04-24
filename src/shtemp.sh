#!/bin/bash

# ./clean-data.sh 
# sleep 1
./script-create-member.sh 
sleep 1
./script-build-cube.sh 
sleep 1
./script-insert-measure-0.sh 
sleep 1
./script-select-0.sh

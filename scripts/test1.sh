#!/bin/bash

sub_pids=()
for j in {1..100}; do
    ./mqtt_sub.o > output.txt &
    sub_pids+=($!)
done
sleep 15
echo -e "Here We Go!\nNow we are Testing\n Let's see\n\n" > ./mqtt_pub
sleep 15 
for k in "${sub_pids[@]}"; do 
    kill -2 "$k"
done

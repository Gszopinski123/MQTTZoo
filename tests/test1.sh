
pids=()
for i in {0..100}; 
do
    ./mqtt_sub.o &
    pids+=($!)
done
sleep 15

for pid in "${pids[@]}" 
do
    kill $pid
done


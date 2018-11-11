#!/bin/sh

cd ./all_maps-scens;
for i in $(ls -d */); do 
map="./all_maps-scens/$i${i%%/}.map";
scen="$map.scen"; 
echo "$i"; 
cd ..;
./wildcard_cut_src_rect pre $map $scen stats.csv;
cd ./all_maps-scens;
done


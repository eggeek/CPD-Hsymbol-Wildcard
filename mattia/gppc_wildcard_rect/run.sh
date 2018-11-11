#!/bin/sh

cd ./maps-scens;
for i in $(ls -d */); do 
map="./maps-scens/$i${i%%/}.map";
scen="$map.scen"; 
echo "$i"; 
cd ..;
./wildcard_cut_src_rect pre $map $scen stats.csv;
cd ./maps-scens;
done


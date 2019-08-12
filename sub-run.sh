#!/usr/bin/env bash
map_names=(
ArcticStation
ca_cavern1_haunted
maze-100-1
orz300d
random-100-33
room-100-10
)

map_dir="./maps/gppc/"
scen_dir="./scens/gppc/"
out_dir="./outputs/cpd/"
centroid=7
order="CUT"


echo "***** run CPD *****"
# run cpd
for i in "${map_names[@]}"; do
  mpath="${map_dir}${i}.map"
  spath="${scen_dir}${i}.map.scen"

  indexpath="./index_data/${i}.map-DFS-3-opt"
  cmd="./bin/main -R -M ${mpath} -S ${spath} -I ${indexpath} -O ${out_dir}${i}-3.txt"
  echo $cmd
  eval $cmd

  indexpath="./index_data/${i}.map-DFS-3-c${centroid}"
  cmd="./bin/main -R -M ${mpath} -S ${spath} -I ${indexpath} -O ${out_dir}${i}-3-c${centroid}.txt"
  echo $cmd
  eval $cmd

  indexpath="./index_data/${i}.map-CUT-3-opt-inv"
  cmd="./bin/main -R -M ${mpath} -S ${spath} -I ${indexpath} -O ${out_dir}${i}-3-cut-inv.txt"
  echo $cmd
  eval $cmd

  indexpath="./index_data/${i}.map-CUT-3-c${centroid}-inv"
  cmd="./bin/main -R -M ${mpath} -S ${spath} -I ${indexpath} -O ${out_dir}${i}-3-cut-inv-c${centroid}.txt"
  echo $cmd
  eval $cmd
done

echo "***** run competitor: tree *****"
# run tree
cd ./competitors/tree

for i in "${map_names[@]}"; do
  mpath="${map_dir}${i}.map"
  spath="${scen_dir}${i}.map.scen"

  cmd="./test -run ${mpath} ${spath} > outputs/${i}.txt"
  echo $cmd
  eval $cmd
done

cd -

echo "***** run competitor: focal *****"
# run focal
for i in "${map_names[@]}"; do
  mpath="${map_dir}${i}.map"
  spath="${scen_dir}${i}.map.scen"
  cmd="./bin/focal ${mpath} ${spath} ${centroid}"
  echo $cmd
  eval $cmd
done

echo "Done."

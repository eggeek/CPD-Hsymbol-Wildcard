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
centroid=7
order="CUT"

echo "***** Preprocess CPD *****"
# precompute cpd
for i in "${map_names[@]}"; do
  mpath="${map_dir}${i}.map"
  spath="${scen_dir}${i}.map.scen"

  cmd="./bin/main -P -M ${mpath} -L 3"
  echo $cmd
  eval $cmd

  cmd="./bin/main -P -M ${mpath} -L 3 --centroid ${centroid}"
  echo $cmd
  eval $cmd

  cmd="./bin/main -P -M ${mpath} -L 3 --order ${order}"
  echo $cmd
  eval $cmd

  cmd="./bin/main -P -M ${mpath} -L 3 --order ${order} --centroid ${centroid}"
  echo $cmd
  eval $cmd
done

echo "***** Preprocess competitor: tree *****"
# precompute tree
cd ./competitors/tree

for i in "${map_names[@]}"; do
  mpath="${map_dir}${i}.map"
  spath="${scen_dir}${i}.map.scen"

  cmd="./test -pre ${mpath} ${spath}"
  echo $cmd
  eval $cmd
done

cd -
echo "Done."

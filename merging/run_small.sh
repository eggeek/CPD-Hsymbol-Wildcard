#!/bin/bash
if [ $# -eq 1 ]
  then
    t=$1
  else
    t="-run"
fi

for i in `ls small_maps/*.map`; do
  for ((h=0; h<=3; h++)) {
    name=$(basename "${i%.*}")
    mpath="small_maps/${name}.map"
    spath="scens/${name}.map.scen"
    cmd="./bin/main $t $h ${mpath} ${spath}"
    echo $cmd
    eval $cmd
  }
done

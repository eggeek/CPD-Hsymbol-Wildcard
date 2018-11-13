#!/bin/bash
if [ $# -eq 1 ]
  then
    t=$1
  else
    t="-run"
fi

for i in `ls maps/*.map`; do
  for ((h=0; h<=2; h++)) {
    name=$(basename "${i%.*}")
    mpath="maps/${name}.map"
    spath="scens/${name}.map.scen"
    cmd="./bin/main $t $h ${mpath} ${spath}"
    echo $cmd
    eval $cmd
  }
done

#!/bin/bash
if [ $# -eq 1 ]
  then
    t=$1
  else
    t="-run"
fi

for ((h=0; h<=3; h++)) {
  for i in `ls maps/dao/*.map`; do
    name=$(basename "${i%.*}")
    mpath="maps/dao/${name}.map"
    spath="scens/dao/${name}.map.scen"
    cmd="./bin/main $t $h ${mpath} ${spath}"
    echo $cmd
    eval $cmd
  done
}

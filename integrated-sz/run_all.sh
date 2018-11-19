#!/bin/bash
if [ $# -eq 1 ]
  then
    t=$1
  else
    t="-run"
fi


for i in `ls -rS maps/*.map`; do
  name=$(basename "${i%.*}")
  mpath="small_maps/${name}.map"
  spath="scens/${name}.map.scen"

  # run l3 + pure
  cmd="./bin/main $t -pure 3 ${mpath} ${spath}"
  echo $cmd
  eval $cmd

  # run l3 + wildcard 
  cmd="./bin/main $t 3 ${mpath} ${spath}"
  echo $cmd
  eval $cmd

  # run wildcard + l0
  cmd="./bin/main $t 0 ${mpath} ${spath}"
  echo $cmd
  eval $cmd

  # run pure
  cmd="./bin/main $t -pure 0 ${mpath} ${spath}"
  echo $cmd
  eval $cmd
done

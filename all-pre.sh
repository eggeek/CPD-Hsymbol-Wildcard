#!/usr/bin/env bash

map_dir="./maps/gppc/"
scen_dir="./scens/gppc/"
order="DFS"

map_names=(`ls ${map_dir}`)

run_cpd() {
  echo "----- Preprocess CPD -----" 
  # precompute cpd
  for i in "${map_names[@]}"; do
    mpath="${map_dir}${i}"
    spath="${scen_dir}${i}.scen"
    for c in `seq 0 2 8`; do
      cmd="./bin/main -P -M ${mpath} -L 3 --centroid ${c}"
      echo $cmd
      eval $cmd
    done
  done
}

run_tree() {
  echo "----- Preprocess competitor: tree -----" 
  # precompute tree
  cd ./competitors/tree

  for i in "${map_names[@]}"; do
    mpath="${map_dir}${i}"
    spath="${scen_dir}${i}.scen"

    cmd="./test -pre ${mpath} ${spath}"
    echo $cmd
    eval $cmd
  done

  cd -
}

run() {
  case "$1" in 
    cpd) run_cpd ;;
    tree) run_tree ;;
    all)
      run_cpd
      run_tree;;
    *) 
      run_cpd
      run_tree;;
  esac
}

run $1
if [ "$1" != "" ]
then
  shift
fi
while [ "$1" != "" ]; do
  run $1
  shift
done

echo "Done."

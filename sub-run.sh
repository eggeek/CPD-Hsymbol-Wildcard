#!/bin/bash
map_names=(
Archipelago
ca_cavern1_haunted
maze-100-1
orz300d
random-100-33
room-100-10
)

map_dir="./maps/gppc/"
scen_dir="./scens/gppc/"
out_dir="./outputs/cpd/"
order="DFS"


run_cpd() {
  echo "----- run CPD -----"
  # run cpd
  for i in "${map_names[@]}"; do
    mpath="${map_dir}${i}.map"
    spath="${scen_dir}${i}.map.scen"

    indexpath="./index_data/${i}.map-DFS-3-opt"
    cmd="./bin/main -R -M ${mpath} -S ${spath} -I ${indexpath} -O ${out_dir}${i}-3-dfs-opt.txt"
    echo $cmd
    eval $cmd

    indexpath="./index_data/${i}.map-DFS-3-opt-inv"
    cmd="./bin/main -R -M ${mpath} -S ${spath} -I ${indexpath} -O ${out_dir}${i}-3-dfs-opt-inv.txt"
    echo $cmd
    eval $cmd

    for c in `seq 2 2 8`; do
      indexpath="./index_data/${i}.map-DFS-3-c${c}"
      cmd="./bin/main -R -M ${mpath} -S ${spath} -I ${indexpath} -O ${out_dir}${i}-3-dfs-c${c}.txt"
      echo $cmd
      eval $cmd

      indexpath="./index_data/${i}.map-DFS-3-c${c}-inv"
      cmd="./bin/main -R -M ${mpath} -S ${spath} -I ${indexpath} -O ${out_dir}${i}-3-dfs-c${c}-inv.txt"
      echo $cmd
      eval $cmd
    done
  done
}

run_tree() {
  echo "----- run competitor: tree -----"
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
}

run_focal() {
  echo "----- run competitor: focal -----"
  # run focal
  for i in "${map_names[@]}"; do
    mpath="${map_dir}${i}.map"
    spath="${scen_dir}${i}.map.scen"
    for c in `seq 0 2 8`; do
      cmd="./bin/focal ${mpath} ${spath} ${c}"
      echo $cmd
      eval $cmd
    done
  done
}

run() {
  case "$1" in 
    cpd) run_cpd ;;
    tree) run_tree ;;
    focal) run_focal ;;
    all)
      run_cpd
      run_tree
      run_focal;;
    *) 
      run_cpd
      run_tree
      run_focal ;;
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

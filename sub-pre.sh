#!/usr/bin/env bash
map_names=(
Aurora
orz103d
maze-400-4
room-400-40
random-400-33
)

map_dir="./maps/gppc/"
scen_dir="./scens/gppc/"
order="DFS"
cs=(64 32 16 8 4 2 0)

run_cpd() {
  echo "----- Preprocess CPD -----" 
  # precompute cpd
  for i in "${map_names[@]}"; do
    mpath="${map_dir}${i}.map"
    spath="${scen_dir}${i}.map.scen"

    for c in "${cs[@]}"; do
      cmd="./bin/main -P -M ${mpath} -L 3 --order ${order} --centroid ${c} -T inv"
      echo $cmd
      eval $cmd
    done

    for c in "${cs[@]}"; do
      cmd="./bin/main -P -M ${mpath} -L 3 --order ${order} --centroid ${c}"
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
    mpath="${map_dir}${i}.map"
    spath="${scen_dir}${i}.map.scen"

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

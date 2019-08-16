#!/bin/bash
map_names=(`ls ${map_dir}`)
map_dir="./maps/gppc/"
scen_dir="./scens/gppc/"
out_dir="./outputs/cpd/"
order="DFS"
cs=(0 2 4 8 16 32)


run_cpd() {
  echo "----- run CPD -----"
  # run cpd
  for i in "${map_names[@]}"; do
    mpath="${map_dir}${i}"
    spath="${scen_dir}${i}.scen"
    for c in "${cs[@]}"; do
      if [ $c -ne 0 ]
        then
          indexpath="./index_data/${i}-DFS-3-c${c}"
          outpath="${out_dir}${i}-3-dfs-c${c}"
        else
          indexpath="./index_data/${i}-DFS-3-opt"
          outpath="${out_dir}${i}-3-dfs-opt"
      fi
      cmd="./bin/main -R -M ${mpath} -S ${spath} -I ${indexpath} -O ${outpath}.txt"
      echo $cmd
      eval $cmd

      cmd="./bin/main -R -M ${mpath} -S ${spath} -I ${indexpath}-inv -O ${outpath}-inv.txt"
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
    mpath="${map_dir}${i}"
    spath="${scen_dir}${i}.scen"

    cmd="./test -run ${mpath} ${spath} > outputs/${i}.txt"
    echo $cmd
    eval $cmd
  done

  cd -
}

focal() {
  echo "$1 `expr 2 '*' $2`"
  eval "$1 `expr 2 '*' $2`"
}

run_focal() {
  echo "----- run competitor: focal -----"
  # run focal
  for i in "${map_names[@]}"; do
    mpath="${map_dir}${i}"
    spath="${scen_dir}${i}.scen"
    cmd="./bin/focal ${mpath} ${spath}"
    export -f focal
    parallel -P 5 focal ::: "${cmd}" ::: "${cs[@]}"
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

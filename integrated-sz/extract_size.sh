#!/bin/bash
header="map,hLevel,pure,bytes"
echo $header
for i in `find ./index_data/ -name "*-RLE-*"`; do
  map=$(basename ${i%.*})
  hlevel="${i: -1}"
  if [[ $i == *"-pure-"* ]]; then 
    pure=1
  else
    pure=0
  fi
  size=$(du -b $i | awk '{print $1}')
  row="$map,$hlevel,$pure,$size"
  echo $row
done;

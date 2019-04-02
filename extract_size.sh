#!/bin/bash
header="map,hLevel,bytes"
echo $header
for i in `find . -name "*-RLE-*"`; do
  map=$(basename ${i%.*})
  hlevel="${i: -1}"
  size=$(du -b $i | awk '{print $1}')
  row="$map,$hlevel,$size"
  echo $row
done;

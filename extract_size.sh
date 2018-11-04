#!/bin/bash
header="map,bytes"
echo $header
for i in `find . -name "*-RLE"`; do
  map=$(basename ${i%.*})
  size=$(du -b $i | awk '{print $1}')
  echo $map,$size
done;

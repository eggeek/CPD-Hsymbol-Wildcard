#!/bin/sh

./bin/main -P -M maps/arena.map -L 3
./bin/main -P -M maps/arena.map -L 3 --centroid
./bin/main -P -M maps/arena.map -L 3 --order CUT
./bin/main -P -M maps/arena.map -L 3 --order CUT --centroid

./bin/main -P -M maps/brc000d.map -L 3
./bin/main -P -M maps/brc000d.map -L 3 --centroid
./bin/main -P -M maps/brc000d.map -L 3 --order CUT
./bin/main -P -M maps/brc000d.map -L 3 --order CUT --centroid

./bin/main -P -M maps/dao/brc101d.map -L 3
./bin/main -P -M maps/dao/brc101d.map -L 3 --centroid
./bin/main -P -M maps/dao/brc101d.map -L 3 --order CUT
./bin/main -P -M maps/dao/brc101d.map -L 3 --order CUT --centroid

#!/bin/sh

./bin/main -P -M maps/arena.map -L 3
./bin/main -P -M maps/arena.map -L 3 --centroid 4
./bin/main -P -M maps/arena.map -L 3 --order FRACTAL
./bin/main -P -M maps/arena.map -L 3 --order FRACTAL --centroid 4

./bin/main -P -M maps/brc000d.map -L 3
./bin/main -P -M maps/brc000d.map -L 3 --centroid 4
./bin/main -P -M maps/brc000d.map -L 3 --order FRACTAL 
./bin/main -P -M maps/brc000d.map -L 3 --order FRACTAL --centroid 4

./bin/main -P -M maps/dao/brc101d.map -L 3
./bin/main -P -M maps/dao/brc101d.map -L 3 --centroid 4
./bin/main -P -M maps/dao/brc101d.map -L 3 --order FRACTAL
./bin/main -P -M maps/dao/brc101d.map -L 3 --order FRACTAL --centroid 4
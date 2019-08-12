#!/bin/sh
./bin/main -R -M maps/arena.map -S scens/arena.map.scen -I ./index_data/arena.map-DFS-3-opt -O arena3.out
./bin/main -R -M maps/arena.map -S scens/arena.map.scen -I ./index_data/arena.map-DFS-3-c7 -O arena3-c7.out
./bin/main -R -M maps/arena.map -S scens/arena.map.scen -I ./index_data/arena.map-CUT-3-opt-inv -O arena3-cut-inv.out
./bin/main -R -M maps/arena.map -S scens/arena.map.scen -I ./index_data/arena.map-CUT-3-c7-inv -O arena3-cut-inv-c7.out

./bin/main -R -M maps/brc000d.map -S scens/brc000d.map.scen -I ./index_data/brc000d.map-DFS-3-opt -O brc000d3.out
./bin/main -R -M maps/brc000d.map -S scens/brc000d.map.scen -I ./index_data/brc000d.map-DFS-3-c7 -O brc000d3-c7.out
./bin/main -R -M maps/brc000d.map -S scens/brc000d.map.scen -I ./index_data/brc000d.map-CUT-3-opt-inv -O brc000d3-cut-inv.out
./bin/main -R -M maps/brc000d.map -S scens/brc000d.map.scen -I ./index_data/brc000d.map-CUT-3-c7-inv -O brc000d3-cut-inv-c7.out

./bin/main -R -M maps/dao/brc101d.map -S scens/dao/brc101d.map.scen -I ./index_data/brc101d.map-DFS-3-opt -O brc101d3.out
./bin/main -R -M maps/dao/brc101d.map -S scens/dao/brc101d.map.scen -I ./index_data/brc101d.map-DFS-3-c7-O brc101d3-c7.out
./bin/main -R -M maps/dao/brc101d.map -S scens/dao/brc101d.map.scen -I ./index_data/brc101d.map-CUT-3-opt-inv -O brc101d3-cut-inv.out
./bin/main -R -M maps/dao/brc101d.map -S scens/dao/brc101d.map.scen -I ./index_data/brc101d.map-CUT-3-c7-inv -O brc101d3-cut-inv-c7.out

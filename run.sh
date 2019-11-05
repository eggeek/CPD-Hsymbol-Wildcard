#!/bin/sh
./bin/main -R -M maps/arena.map -S scens/arena.map.scen -I ./index_data/arena.map-DFS-3-opt -O arena3.out
./bin/main -R -M maps/arena.map -S scens/arena.map.scen -I ./index_data/arena.map-DFS-3-c4 -O arena3-c4.out
./bin/main -R -M maps/arena.map -S scens/arena.map.scen -I ./index_data/arena.map-DFS-3-opt-inv -O arena3-dfs-inv.out
./bin/main -R -M maps/arena.map -S scens/arena.map.scen -I ./index_data/arena.map-DFS-3-c4-inv -O arena3-dfs-inv-c4.out

./bin/main -R -M maps/brc000d.map -S scens/brc000d.map.scen -I ./index_data/brc000d.map-DFS-3-opt -O brc000d3.out
./bin/main -R -M maps/brc000d.map -S scens/brc000d.map.scen -I ./index_data/brc000d.map-DFS-3-c4 -O brc000d3-c4.out
./bin/main -R -M maps/brc000d.map -S scens/brc000d.map.scen -I ./index_data/brc000d.map-DFS-3-opt-inv -O brc000d3-cut-inv.out
./bin/main -R -M maps/brc000d.map -S scens/brc000d.map.scen -I ./index_data/brc000d.map-DFS-3-c4-inv -O brc000d3-cut-inv-c4.out

./bin/main -R -M maps/dao/brc101d.map -S scens/dao/brc101d.map.scen -I ./index_data/brc101d.map-DFS-3-opt -O brc101d3.out
./bin/main -R -M maps/dao/brc101d.map -S scens/dao/brc101d.map.scen -I ./index_data/brc101d.map-DFS-3-c4 -O brc101d3-c4.out
./bin/main -R -M maps/dao/brc101d.map -S scens/dao/brc101d.map.scen -I ./index_data/brc101d.map-DFS-3-opt-inv -O brc101d3-dfs-inv.out
./bin/main -R -M maps/dao/brc101d.map -S scens/dao/brc101d.map.scen -I ./index_data/brc101d.map-DFS-3-c4-inv -O brc101d3-dfs-inv-c4.out

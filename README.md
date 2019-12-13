* `./index_data`: stores all precomputed CPDs (untracked)
  * index file names are in format `{mapname}-{order}-{int}-opt` or `{mapname}-{order}-{int}-c{int}`, where 
  * `{mapname}` is the name of the map, e.g. `AcrosstheCape.map`
  * `{order}` is the ordering, e.g. `DFS` or `CUT`
  * `{int}` indicates the heuristic level, usually is `3`
  * `c{int}` indicates the size of centroid, e.g. `c3` means the radius of centroid is 3
  * `opt` indicates it's a full optimal CPD (size of centroid is `0`)

* `all-pre.sh`: precompute all index
* `all-run.sh`: run all scenarios with precomputed index

* run test: `./bin/test`
* examples of run a single scenario
  * `./bin/main -F -M maps/arena.map -S scens/arena.map.scen -L 3`: precompute and run
  * `./bin/main -P -M maps/arena.map -L 3`: precompute only
  * `./bin/main -R -M maps/arena.map -S scens/arena.map.scen`: run with precomputed index

* `shizhe/index_data`: stores all precomputed CPDs
  * index file names are in format `*-RLE-{int}`, where `{int}` indicates the heuristic level
    of H symbol
    * `*-RLE-0`: no H symbol
    * `*-RLE-1`: simple heuristic function

* `extract_size.sh`: extracts the size of each index file and output by stdout
* `run_all.sh`: run all scenarios with index, results are stored in
  `output/{mapname}-{int}.txt`
  * where `{int}` indicates the heuristic level

* run test: `./bin/test`
* examples of run a single scenario
  * `./bin/main -run maps/arena.map scens/arena.map.scen`
  * `./bin/main -run 0 maps/arena.map scens/arena.map.scen` (heuristic level is `0`)
  * `./bin/main -run 1 maps/arena.map scens/arena.map.scen` (heuristic level is `1`)
  * `./bin/main -full maps/arena.map scens/arena.map.scen` (create index and run)
  * `./bin/main -pre maps/arena.map scens/arena.map.scen` (only create index)


#!/bin/sh

# -DUSE_PARALLELISM causes the preprocessing to use as many cores as available.

# -DDETAILS to get time and number of step_first_move during build full path


#DETAILS
#g++ -static -ldl -std=c++0x -O3 -fopenmp -DNDEBUG -DUSE_PARALLELISM -DDETAILS  *.cpp -o wildcard_detailed_dfs_src
#g++ -static -ldl -std=c++0x -O3 -fopenmp -DNDEBUG -DUSE_PARALLELISM -DDETAILS -DUSE_CUT_ORDER *.cpp -lmetis -o wildcard_detailed_cut_src

#NO DETAILS
#g++ -static -ldl -std=c++0x -O3 -fopenmp -DNDEBUG -DUSE_PARALLELISM *.cpp -o wildcard_dfs_src_rect
g++ -static -ldl -std=c++0x -O3 -g -I./third_party -I./src -I./utility -I./test -fopenmp -DNDEBUG -DUSE_PARALLELISM -DUSE_CUT_ORDER  ./src/*.cpp ./third_party/*.cpp ./utility/*.cpp ./*.cpp -lmetis -o wildcard_cut_src_square


cmake_minimum_required(VERSION 3.10)
project(CPD-Hsymbol-Wildcard)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "-DUSE_PARALLELISM -DEXTRACT_ALL_AT_ONCE")
if(APPLE)
  find_library(metis_lib metis "/usr/local/opt" )
  find_library(omp_lib omp "/usr/local/opt")
else (APPLE)
  set(metis_lib "/home/szha414/metis/lib/libmetis.a")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -I/home/szha414/metis/include -fopenmp -lboost_program_options")
endif(APPLE)
set(DEV_CXX_FLAGS "-g -ggdb -O0 -fno-omit-frame-pointer")
set(RELEASE_CXX_FLAGS "-O3 -DNDEBUG")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${DEV_CXX_FLAGS}")
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/../bin)

include_directories(src)
include_directories(tests)
include_directories(third_party)
include_directories(utility)

# config for source code
set(SOURCE_FILES
    src/adj_array.h
    src/adj_graph.h
    src/dijkstra.h
    src/preprocessing.h
    src/preprocessing.cpp
    src/loader.h
    src/query.h
    src/query.cpp
    src/Hsymbol.h
    src/list_graph.h
    src/order.cpp
    src/order.h
    src/square_wildcard.h
    src/rect_wildcard.h
    third_party/cpd.cpp
    third_party/cpd.h
    third_party/gm_parser.cpp
    third_party/gm_parser.h
    third_party/gridmap.cpp
    third_party/gridmap.h
    third_party/jps.cpp
    third_party/jps.h
    third_party/jpsp_oracle.cpp
    third_party/jpsp_oracle.h
    third_party/offline_jump_point_locator.cpp
    third_party/offline_jump_point_locator.h
    third_party/online_jump_point_locator.cpp
    third_party/online_jump_point_locator.h
    utility/binary_search.h
    utility/constants.h
    utility/heap.h
    utility/mapper.cpp
    utility/mapper.h
    utility/range.h
    utility/ScenarioLoader.cpp
    utility/ScenarioLoader.h
    utility/timer.cpp
    utility/timer.h
    utility/vec_io.h
    utility/visualizer.h
    # cut ordering
    utility/balanced_min_cut.cpp
    utility/balanced_min_cut.h
    utility/cut_utility.h
    utility/cut_utility.cpp
    utility/prefer_zero_cut.h
    utility/prefer_zero_cut.cpp
    )

add_executable(main
        ${SOURCE_FILES}
        main.cpp)

find_package(Boost COMPONENTS program_options REQUIRED )
if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
    target_link_libraries(main ${Boost_LIBRARIES})
endif()

# config for tests
set(TESTING_FILES
    tests/test-operations.cpp
    tests/test-heuristic.cpp
    tests/test-visual.cpp
    tests/test-preprocess.cpp
    tests/test-query.cpp
    tests/catch.hpp)


add_executable(testing
        ${SOURCE_FILES}
        ${TESTING_FILES}
        tests/main.cpp)

if (omp_lib)
  target_link_libraries(main ${omp_lib})
  target_link_libraries(testing ${omp_lib})
endif()

if (metis_lib)
  target_link_libraries(main ${metis_lib})
  target_link_libraries(testing ${metis_lib})
endif()


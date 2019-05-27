cmake_minimum_required(VERSION 3.12)
project(CPD-Hsymbol-Wildcard)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "-L/usr/local/opt/libomp/lib -I/usr/local/opt/libomp/include -DUSE_PARALLELISM -DEXTRACT_ALL_AT_ONCE -lomp")
set(DEV_CXX_FLAGS "-g -ggdb -O0 -fno-omit-frame-pointer")
set(RELEASE_CXX_FLAGS "-O3 -DNDEBUG")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${DEV_CXX_FLAGS}")
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/../bin)

include_directories(src)
include_directories(tests)
include_directories(third_party)
include_directories(utility)


set(SOURCE_FILES
    src/adj_array.h
    src/adj_graph.h
    src/dijkstra.h
    src/Entry.cpp
    src/Entry.h
    src/Hsymbol.h
    src/list_graph.h
    src/order.cpp
    src/order.h
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
    utility/Timer.cpp
    utility/Timer.h
    utility/vec_io.h
    utility/visualizer.h)

set(TESTING_FILES
    tests/test-operations.cpp
    tests/test-heuristic.cpp
    tests/catch.hpp)

add_executable(main
        ${SOURCE_FILES}
        main.cpp)

target_link_libraries(main ${OpenMP_omp_LIBRARY})

add_executable(testing
        ${SOURCE_FILES}
        ${TESTING_FILES}
        testing.cpp)

target_link_libraries(testing ${OpenMP_omp_LIBRARY})

CXX = g++
CFLAGS = -ldl -std=c++0x -Xpreprocessor -fopenmp -lomp -DUSE_PARALLELISM -DEXTRACT_ALL_AT_ONCE
FAST_FLAGS = -O3 -DNDEBUG 
DEV_FLAGS = -g -ggdb -O0 -fno-omit-frame-pointer
DIRS = src third_party utility
INCLUDES = $(addprefix -I,$(DIRS))
SRC = $(foreach folder,$(DIRS),$(wildcard $(folder)/*.cpp))
SRC += $(wildcard *.cpp)
OBJ = $(SRC:.cpp=.o)


src_jps_dfs: ${OBJ} 
	$(CXX) ${CFLAGS} ${INCLUDES} $(addprefix obj/,$(notdir $(OBJ))) -o bin/$@ 

%.o: %.cpp
	$(CXX) $(CFLAGS) ${INCLUDES} -c $< -o obj/$(notdir $@)


clean:
	rm obj/*o bin/*

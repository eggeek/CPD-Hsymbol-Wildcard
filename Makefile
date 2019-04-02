MAKEFLAGS += -r
PA_FOLDERS = src utility third_party test 
PA_SRC = $(foreach folder,$(PA_FOLDERS),$(wildcard $(folder)/*.cpp))
PA_OBJ = $(PA_SRC:.cpp=.o)
PA_INCLUDES = $(addprefix -I,$(PA_FOLDERS))


CXX = g++
CXXFLAGS = -std=c++0x -Xpreprocessor -fopenmp -lomp -DUSE_PARALLELISM -DEXTRACT_ALL_AT_ONCE
FAST_CXXFLAGS = -O3 -DNDEBUG 
DEV_CXXFLAGS = -g -ggdb -O0 -fno-omit-frame-pointer

TARGETS = test main 
BIN_TARGETS = $(addprefix bin/,$(TARGETS))

all: $(TARGETS)
fast: CXXFLAGS += $(FAST_CXXFLAGS)
dev: CXXFLAGS += $(DEV_CXXFLAGS)
fast dev: all

clean:
	rm -rf ./bin/*
	rm -f $(PA_OBJ:.o=.d)
	rm -f $(PA_OBJ)

clear:
	find . -name '*.jps+' -delete

.PHONY: $(TARGETS)
$(TARGETS): % : bin/%

$(BIN_TARGETS): bin/%: %.cpp $(PA_OBJ)
	@mkdir -p ./bin
	$(CXX) $(CXXFLAGS) $(PA_INCLUDES) $(PA_OBJ) $(@:bin/%=%).cpp -o $(@)

-include $(PA_OBJ:.o=.d)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(PA_INCLUDES) -MM -MP -MT $@ -MF ${@:.o=.d} $<
	$(CXX) $(CXXFLAGS) $(PA_INCLUDES) $< -c -o $@

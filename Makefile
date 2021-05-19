build_dir="cmake-build"
flag="Debug"

all: build tree
fast dev: all

dev: flag = Debug
fast: flag = Release

.PHONY: gen build clean
gen:
	@mkdir -p ${build_dir}
	@echo "cmake -B${build_dir} -H. -DCMAKE_BUILD_TYPE=${flag} -DCMAKE_EXPORT_COMPILE_COMMANDS=1"
	@eval "cmake -B${build_dir} -H. -DCMAKE_BUILD_TYPE=${flag} -DCMAKE_EXPORT_COMPILE_COMMANDS=1"

build: gen
	@echo "cd ${build_dir} && make -j8"
	@eval "cd ${build_dir} && make -j8"

clean:
	@echo "cd ${build_dir} && make clean"
	@eval "cd ${build_dir} && make clean"

tree:
	@echo "cd ./competitors/tree && make compile"
	@eval "cd ./competitors/tree && make compile"

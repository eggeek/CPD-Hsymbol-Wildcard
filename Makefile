build_dir="cmake-build"
all: dev
dev:
	@echo "cmake -B${build_dir} -H. -DCMAKE_BUILD_TYPE=Debug"
	@eval "cmake -B${build_dir} -H. -DCMAKE_BUILD_TYPE=Debug"

	@echo "cd ${build_dir} && make -j8"
	@eval "cd ${build_dir} && make -j8"

fast:
	@echo "cmake -B${build_dir} -H. -DCMAKE_BUILD_TYPE=Release"
	@eval "cmake -B${build_dir} -H. -DCMAKE_BUILD_TYPE=Release"

	@echo "cd ${build_dir} && make -j8"
	@eval "cd ${build_dir} && make -j8"

clean:
	@echo "cd ${build_dir} && make clean"
	@eval "cd ${build_dir} && make clean"


compile: 
	g++ -O3 -Wall -DTEST_FOR_PATH_CONNECTIVITY -o test main.cpp ScenarioLoader.cpp Timer.cpp Entry.cpp
#	g++ -O3 -Wall -o test main.cpp ScenarioLoader.cpp Timer.cpp Entry.cpp

dev:
	g++ -g -ggdb -O0 -fno-omit-frame-pointer -Wall -DTEST_FOR_PATH_CONNECTIVITY -o test main.cpp ScenarioLoader.cpp Timer.cpp Entry.cpp

run: 
#	./test -full rmtst01.map rmtst01.map.scen
#	./test -pre AcrosstheCape.map AcrosstheCape.map.scen
#	./test -run AcrosstheCape.map AcrosstheCape.map.scen
	./test -full lak507d.map lak507d.map.scen

clean:
	rm test


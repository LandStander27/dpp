inp = "default"

build: $(wildcard src/*.cpp)
	mkdir -p bin
	g++ -std=c++20 -O3 -static -Iinclude -o bin/d++ $^
	rm -rf bin/include
	mkdir -p bin/include
	cp -r ./stdlib/* ./bin/include

windows-build: $(wildcard src/*.cpp)
	mkdir -p bin
	g++.exe -std=c++20 -O3 -static -Iinclude -o bin/d++.exe $^
	rm -rf bin/include
	mkdir -p bin/include
	cp -r ./stdlib/* ./bin/include

run:
	@make build
	./bin/d++ $(inp)

debug: $(wildcard src/*.cpp)
	mkdir -p bin
	g++ -std=c++20 -Iinclude -o bin/d++ $^
	rm -rf bin/include
	mkdir -p bin/include
	cp -r ./stdlib/* ./bin/include

windows-debug: $(wildcard src/*.cpp)
	mkdir -p bin
	g++.exe -std=c++20 -Iinclude -o bin/d++.exe $^
	rm -rf bin/include
	mkdir -p bin/include
	cp -r ./stdlib/* ./bin/include

clean:
	rm -rf bin

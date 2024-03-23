inp = "default"

build: $(wildcard src/*.cpp)
	mkdir -p bin
	g++ -std=c++20 -O3 -static -Iinclude -o bin/d++ $^
	@make copy-builtin

copy-builtin:
	mkdir -p bin
	rm -rf bin/include
	mkdir -p bin/include
	cp -r ./builtin/* ./bin/include

windows-build: $(wildcard src/*.cpp)
	mkdir -p bin
	g++.exe -std=c++20 -O3 -static -Iinclude -o bin/d++.exe $^
	@make copy-builtin

run:
	@make build
	./bin/d++ $(inp)

debug: $(wildcard src/*.cpp)
	mkdir -p bin
	g++ -std=c++20 -Iinclude -o bin/d++ $^
	@make copy-builtin

windows-debug: $(wildcard src/*.cpp)
	mkdir -p bin
	g++.exe -std=c++20 -Iinclude -o bin/d++.exe $^
	@make copy-builtin

clean:
	rm -rf bin

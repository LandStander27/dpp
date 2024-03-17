inp = "default"

build: $(wildcard src/*.cpp)
	mkdir -p bin
	g++ -std=c++20 -static -Iinclude -o bin/d++ $^

run:
	@make build
	./bin/d++ $(inp)

debug: $(wildcard src/*.cpp)
	mkdir -p bin
	g++ -std=c++20 -Iinclude -o bin/d++ $^

clean:
	rm -rf bin

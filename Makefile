inp = "default"

build: $(wildcard src/*.cpp)
	mkdir -p bin
	g++ -static -Iinclude -o bin/d++ $^

run:
	@make build
	./bin/d++ $(inp)

clean:
	rm -rf bin

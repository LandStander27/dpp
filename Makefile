inp = "default"
run:
	@make build
	./bin/main $(inp)

build: $(wildcard src/*.cpp)
	mkdir -p bin
	g++ -Iinclude -o bin/main $^

clean:
	rm -rf bin

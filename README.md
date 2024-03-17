# D++

Extremely simple compiled language.
- Translates into C++ first and then into an executable. This allows you to put C++ in your D++ code.
- Better type names (i32, i64, u32, f32, etc)

## Getting Started

### Usage

Examples:

- Hello world
```
i32 main() {
	println("Hello, world!"); // Hello, world!
}
```

- String formatting
```
i32 main() {
	i32 a = 6;
	str s = "Mine: {a}";
	println("Number: {s}"); // Number: Mine: 6
}
```

- Vectors
```
i32 main() {
	Vec<i32> a = { 1, 2 };
	a << 3;
	println("Vector: {a}"); // { 1, 2, 3 }
}
```

- Command line arguments
```
i32 main(Vec<str> args) {
	if (args.len() > 1) {
		println("First argument: {args[1]}.");
	} else {
		println("No arguments.");
	}
}
```

- For loops
```
i32 main() {
	for (i32 i in 0..10) {
		print("{i}, ");
	} // 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,

	for (i32 i in 0..=10) {
		print("{i}, ");
	} // 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10,

	Vec<str> a = { "one", "two", "three" };
	for (i32& i in a) {
		print("{i}, ");
	} // one, two, three,
}
```
- Everything else is exactly like C++
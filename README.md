# D++

Extremely simple compiled language.
- Translates into C++ first and then into an executable. This allows you to put C++ in your D++ code. (This is a side effect but I'm calling it a feature).
- Better type names (i32, i64, u32, f32, etc).

## Getting Started

### Usage

Examples:

- Hello world
```
i32 main() {
	println("Hello, world!"); // Hello, world!
}
```

- Stdin
```
i32 main() {
	print("String: ");
	str a = scan();
	println(a);

	print("Integer: ");
	i32 b = scan<i32>(); // Just using scan() should also work.
	println<i32>(b);
}
```

- String formatting
```
i32 main() {
	i32 a = 6;
	str s = "Mine: {a}"; // Calls a.display() and embeds into string. This allows you to create a custom string formatter on an object.
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

	print("\n");

	for (i32 i in 0..=10) {
		print("{i}, ");
	} // 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10,

	print("\n");

	Vec<str> a = { "one", "two", "three" };
	for (str& i in a) {
		print("{i}, ");
	} // one, two, three,

	print("\n");
}
```

- Panics
```
i32 main() {
	f64 first = scan();
	f64 second = scan();

	if (second == 0) {
		panic("Cannot divide by zero.");
	}

	println("First / Second: {first/second}");
}
```

- Everything else is exactly like C++
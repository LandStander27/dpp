#pragma once

#include <iostream>
#include <fstream>
#include <filesystem>
#include <sstream>
#include <string>
#include <vector>
#include <new>
#include <optional>
#include <regex>
#include <chrono>
#include <cmath>

#include <unistd.h>

#ifndef _WIN32

#include <limits.h>
#include <unistd.h>
std::string get_binary_path() {
	char result[PATH_MAX];
	ssize_t count = readlink( "/proc/self/exe", result, PATH_MAX );
	std::string n = std::string(result, (count > 0) ? count : 0);
	return n.substr(0, n.find_last_of("/"));
}
#else

#undef UNICODE
#include <windows.h>

std::string get_binary_path() {
	char result[MAX_PATH];
	std::string n = std::string(result, GetModuleFileName(NULL, result, MAX_PATH));
	return n.substr(0, n.find_last_of("\\"));
}

#endif

#define RESET "\x1b[0m"
#define RED "\x1b[31m"
#define DARKRED "\x1b[91m"
#define GREEN "\x1b[92m"
#define BLUE "\x1b[94m"

#define LOG(x) std::cout << BLUE << "[INFO ] " << x << RESET << std::endl

const char* default_header =
"#include <iostream>" "\n"
"#include <sstream>\n" "\n"

"template <typename T>" "\n"
"class Number;" "\n"
"typedef Number<unsigned long long> u64;" "\n"

"class str {" "\n"
"	std::string val;\n" "\n"

"public:" "\n"
"	str() {" "\n"
"		this->val = \"\";" "\n"
"	}" "\n"
"	template <typename T>" "\n"
"	str(T val) {" "\n"
"		this->val = val.to_string().data();" "\n"
"	}" "\n"
"	str(const std::string val) {" "\n"
"		this->val = val;" "\n"
"	}" "\n"
"	str(char* val) {" "\n"
"		this->val = val;" "\n"
"	}" "\n"
"	str(const char* val) {" "\n"
"		this->val = val;" "\n"
"	}" "\n"
"	str(const char val) {" "\n"
"		this->val = val;" "\n"
"	}" "\n"
"	template <typename T = u64>" "\n"
"	inline T len() {" "\n"
"		return T(this->val.size());" "\n"
"	}" "\n"
"	inline str display() {" "\n"
"		return str(this->val);" "\n"
"	}" "\n"
"	inline const char* c_str() {" "\n"
"		return this->val.c_str();" "\n"
"	}" "\n"
"	str operator[](unsigned long long i) {" "\n"
"		return str(this->val[i]);" "\n"
"	}" "\n"
"	str operator+(const str& other) {" "\n"
"		return str(this->val + other.val);" "\n"
"	}" "\n"
"	str operator+(const char* other) {" "\n"
"		return str(this->val + other);" "\n"
"	}" "\n"
"	bool operator==(const str& other) {" "\n"
"		return this->val == other.val;" "\n"
"	}" "\n"
"	std::string data() {" "\n"
"		return this->val;" "\n"
"	}" "\n"
"	char* begin() { return &(this->val[0]); }" "\n"
"	char* end() { return &(this->val[this->val.size()]); }\n" "\n"

"	friend std::ostream& operator<<(std::ostream& os, str str);" "\n"
"};\n" "\n"

"str operator+(const char* a, const str& b) {" "\n"
"	return str(a) + b;" "\n"
"}\n" "\n"

"std::ostream& operator<<(std::ostream& os, str str) {" "\n"
"	os << str.val;" "\n"
"	return os;" "\n"
"}\n" "\n"

"template <typename T>" "\n"
"class Number {" "\n"
"	T val;\n" "\n"

"public:" "\n"
"	Number(T val) {" "\n"
"		this->val = val;" "\n"
"	}\n" "\n"

"	Number() {" "\n"
"		this->val = 0;" "\n"
"	}\n" "\n"

"	Number(const str& val) {" "\n"
"		std::stringstream ss;" "\n"
"		ss << val;" "\n"
"		ss >> this->val;" "\n"
"	}\n" "\n"

"	inline T& data() {" "\n"
"		return this->val;" "\n"
"	}\n" "\n"

"	str to_string() {" "\n"
"		return str(std::to_string(this->val));" "\n"
"	}\n" "\n"

"	str display() {" "\n"
"		return str(*this);" "\n"
"	}\n" "\n"

"	inline bool operator ==(Number other) { return this->val == other.val; }" "\n"
"	inline bool operator ==(T other) { return this->val == other; }" "\n"
"	inline bool operator !=(Number other) { return this->val != other.val; }" "\n"
"	inline bool operator !=(T other) { return this->val != other; }" "\n"

"	inline bool operator <(Number other) { return this->val < other.val; }" "\n"
"	inline bool operator <(T other) { return this->val < other; }" "\n"
"	inline bool operator >(Number other) { return this->val > other.val; }" "\n"
"	inline bool operator >(T other) { return this->val > other; }" "\n"
"	inline bool operator <=(Number other) { return this->val <= other.val; }" "\n"
"	inline bool operator <=(T other) { return this->val <= other; }" "\n"
"	inline bool operator >=(Number other) { return this->val >= other.val; }" "\n"
"	inline bool operator >=(T other) { return this->val >= other; }" "\n"

"	inline void operator =(T other) { this->val = other; }\n" "\n"

"	inline void operator -=(Number other) { this->val -= other.val; }" "\n"
"	inline void operator +=(Number other) { this->val += other.val; }" "\n"
"	inline void operator /=(Number other) { this->val /= other.val; }" "\n"
"	inline void operator *=(Number other) { this->val *= other.val; }\n" "\n"

"	inline Number operator -(Number other) { return this->val - other.val; }" "\n"
"	inline Number operator +(Number other) { return this->val + other.val; }" "\n"
"	inline Number operator /(Number other) { return this->val / other.val; }" "\n"
"	inline Number operator *(Number other) { return this->val * other.val; }\n" "\n"

"	inline T operator -(T other) { return this->val - other; }" "\n"
"	inline T operator +(T other) { return this->val + other; }" "\n"
"	inline T operator /(T other) { return this->val / other; }" "\n"
"	inline T operator *(T other) { return this->val * other; }\n" "\n"

"	inline void operator ++() { this->val++; }" "\n"
"	inline void operator --() { this->val--; }" "\n"

"	inline void operator ++(int) { this->val++; }" "\n"
"	inline void operator --(int) { this->val--; }" "\n"

"};\n" "\n"

"template <typename T>" "\n"
"Number<T> operator-(T a, Number<T> b) {" "\n"
"	return Number(a) - b;" "\n"
"}\n" "\n"

"template <typename T>" "\n"
"Number<T> operator+(T a, Number<T> b) {" "\n"
"	return Number(a) + b;" "\n"
"}\n" "\n"

"template <typename T>" "\n"
"Number<T> operator/(T a, Number<T> b) {" "\n"
"	return Number(a) / b;" "\n"
"}\n" "\n"

"template <typename T>" "\n"
"Number<T> operator*(T a, Number<T> b) {" "\n"
"	return Number(a) * b;" "\n"
"}\n" "\n"

"template <typename T>" "\n"
"Number<T> operator<(T a, Number<T> b) {" "\n"
"	return Number(a) < b;" "\n"
"}\n" "\n"

"template <typename T>" "\n"
"Number<T> operator>(T a, Number<T> b) {" "\n"
"	return Number(a) > b;" "\n"
"}\n" "\n"

"template <typename T>" "\n"
"std::string to_string(Number<T>& n) {" "\n"
"	return std::to_string(n.data());" "\n"
"}\n" "\n"
// "class i32 : public Number<int> {" "\n"
// "public:" "\n"
// "	using Number::Number;" "\n"
// "};\n" "\n"

// "class i64 : public Number<long long> {" "\n"
// "public:" "\n"
// "	using Number::Number;" "\n"
// "};\n" "\n"

// "class u32 : public Number<unsigned int> {" "\n"
// "public:" "\n"
// "	using Number::Number;" "\n"
// "};\n" "\n"

// "class u64 : public Number<unsigned long long> {" "\n"
// "public:" "\n"
// "	using Number::Number;" "\n"
// "};\n" "\n"

// "class f32 : public Number<float> {" "\n"
// "public:" "\n"
// "	using Number::Number;" "\n"
// "};\n" "\n"

// "class f64 : public Number<double> {" "\n"
// "public:" "\n"
// "	using Number::Number;" "\n"
// "};\n" "\n"

"typedef Number<int> i32;" "\n"
"typedef Number<long long> i64;" "\n"
"typedef Number<unsigned int> u32;" "\n"
"typedef Number<unsigned long long> u64;" "\n"
"typedef Number<float> f32;" "\n"
"typedef Number<double> f64;\n" "\n"

"template <typename T>" "\n"
"static inline void println(T s) { std::cout << T(s).display() << std::endl; }" "\n"
"template <typename T>" "\n"
"static inline void print(T s) { std::cout << T(s).display(); }\n" "\n"

"static inline void println(const char* s) { std::cout << s << std::endl; }" "\n"
"static inline void print(const char* s) { std::cout << s; }\n" "\n"

"template <typename T = str>" "\n"
"T scan() { std::string t; std::getline(std::cin, t); return T(t); }\n" "\n"

"[[noreturn]] void panic(str msg) { std::cout << \"\\x1b[31m\" << \"[PANIC]: \" << msg << \"\\x1b[0m\" << std::endl; exit(101); }" "\n"
"[[noreturn]] void panic(str msg, int line, str file) { std::cout << \"\\x1b[31m\" << \"[PANIC]: \" << msg << \"\\n\\t--> \" << file << \":\" << line << \"\\x1b[0m\" << std::endl; exit(101); }\n" "\n";

#include "defs.hpp"
#include "argparse/argparse.hpp"

extern char **environ;

const std::string vec_impl =
"template<typename T, typename Alloc = std::allocator<T>>" "\n"
"class Vec {" "\n"
"	std::vector<T> v;" "\n"
"public:" "\n"
"	Vec(std::initializer_list<T> init) {" "\n"
"		this->v = std::vector<T>(init);" "\n"
"	}" "\n"
"	Vec() {" "\n"
"		this->v = std::vector<T>();" "\n"
"	}" "\n"
"	Vec(T val, u64 len) {" "\n"
"		this->v = std::vector<T>();" "\n"
"		this->v.reserve(len);" "\n"
"		for (int i = 0; i < len; i++) {" "\n"
"			this->v.push_back(val);" "\n"
"		}" "\n"
"	}" "\n"
"	inline u64 len() {" "\n"
"		return this->v.size();" "\n"
"	}" "\n"
"	std::string display() {" "\n"
"		std::stringstream os;" "\n"
"		os << \"{ \";" "\n"
"		for (int i = 0; i < this->len(); i++) {" "\n"
"			os << (*this)[i].display();" "\n"
"			if (i < this->len() - 1) {" "\n"
"				os << \", \";" "\n"
"			}" "\n"
"		}" "\n"
"		os << \" }\";" "\n"
"		return os.str();" "\n"
"	}" "\n"
"	bool contains(T val) {" "\n"
"		for (int i = 0; i < this->len(); i++) {" "\n"
"			if (this->v[i] == val) {" "\n"
"				return true;" "\n"
"			}" "\n"
"		}" "\n"
"		return false;" "\n"
"	}" "\n"
"	void operator<<(T t) {" "\n"
"		this->v.push_back(t);" "\n"
"	}" "\n"
"	template<typename U>" "\n"
"	T& operator[](U i) {" "\n"
"		if (i >= this->len()) {" "\n"
"			panic(\"Index out of bounds: len is \" + this->len().to_string() + \", index is \" + std::to_string(i));" "\n"
"		}" "\n"
"		return this->v[i];" "\n"
"	}\n" "\n"

"	T* begin() { return &(this->v[0]); }" "\n"
"	T* end() { return &(this->v[this->len()]); }" "\n"
"};\n" "\n";

// "template<typename T>" "\n"
// "std::ostream& operator<<(std::ostream& os, Vec<T>& vec) {" "\n"
// "	os << \"{ \";" "\n"
// "	for (int i = 0; i < vec.len(); i++) {" "\n"
// "		os << vec[i];" "\n"
// "		if (i < vec.len() - 1) {" "\n"
// "			os << \", \";" "\n"
// "		}" "\n"
// "	}" "\n"
// "	os << \" }\";" "\n"
// "	return os;" "\n"
// "}\n";

#define LOG_IF_V(x) if (parser["--verbose"] == true) { LOG(x); }

std::optional<std::string> format_string(std::string org) {

	std::stringstream ss;

	ss << "({";

	int string_start = org.find("\"");
	ss << "std::stringstream no_one_name_your_variable_this; ";
	ss << "no_one_name_your_variable_this << ";
	for (int i = string_start; i < org.size()-2; i++) {
		if (org[i] == '{' && (i != org.size()-3 ? org[i+1] != '{' : true)) {
			int end = org.find('}', i);
			if (end == std::string::npos) {
				return std::nullopt;
			}

			ss << "\" << (" << org.substr(i+1, end-i-1) << ").display() << \"";
			i = end;
			continue;
		} else if (org[i] == '{') {
			i += 1;
		}
		ss << org[i];
	}
	ss << "\"; str(no_one_name_your_variable_this.str());})";

	return ss.str();

}

std::vector<int> find_quotes(std::string s) {
	std::vector<int> ret;

	for (int i = 0; i < s.size(); i++) {
		if (s[i] == '\"' && (i != 0 ? s[i-1] != '\\' : true)) {
			ret.push_back(i);
		}
	}

	return ret;
}

int main(int argc, char** argv) {

	argparse::ArgumentParser parser = argparse::ArgumentParser("D++ compiler");

	parser.add_argument("input_file").help("Input file").required();
	parser.add_argument("-o", "--output").help("Output file").default_value(std::string("a.out"));
	parser.add_argument("-kc", "--keep-cpp").help("Keep the translated C++").default_value(false).implicit_value(true);
	parser.add_argument("-v", "--verbose").help("Verbose output").default_value(false).implicit_value(true);
	parser.add_argument("-prod").help("Heavy optimization").default_value(false).implicit_value(true);
	parser.add_argument("-s", "--static").help("Static linking").default_value(false).implicit_value(true);

	try {
		parser.parse_args(argc, argv);
	} catch (const std::exception& err) {
		std::cout << err.what() << std::endl;
		std::cout << parser;
		return 1;
	}

	std::string input_file = parser.get<std::string>("input_file");

	if (!std::filesystem::exists(input_file)) {
		std::cout << "Input file does not exist" << std::endl;
		return 2;
	}

	if (!std::filesystem::is_regular_file(input_file)) {
		std::cout << "Invalid file" << std::endl;
		return 2;
	}

	LOG_IF_V("Opening file " << input_file);

	std::ifstream file = std::ifstream(input_file);

	// unsigned int size = std::filesystem::file_size(input_file);
	// std::string input = std::string(size, '\0');
	// file.read(&input[0], size);

	std::stringstream ss;
	file >> std::noskipws;

	bool has_vecs = false;

	std::chrono::duration<double> start = std::chrono::system_clock::now().time_since_epoch();

	LOG_IF_V("Reading file " << input_file);

	int size = std::filesystem::file_size(input_file);
	std::string* entire_file = new std::string(size, '\0');
	file.read(entire_file->data(), size);
	file.seekg(0, file.beg);

	LOG_IF_V("Testing Vec existence");

	if (entire_file->find("Vec") != std::string::npos) {
		has_vecs = true;
	}

	free(entire_file);

	LOG_IF_V("Creating header");

	std::stringstream output;
	output <<
"#include <iostream>" "\n";
if (has_vecs) { output << "#include <vector>" "\n"; }
	output <<
"#include <sstream>\n" "\n"

"class u64;" "\n"

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

"	operator T() {" "\n"
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
"	inline void operator =(T other) { this->val = other; }\n" "\n"

"	inline void operator -=(Number other) { this->val -= other.val; }" "\n"
"	inline void operator +=(Number other) { this->val += other.val; }" "\n"
"	inline void operator /=(Number other) { this->val /= other.val; }" "\n"
"	inline void operator *=(Number other) { this->val *= other.val; }\n" "\n"

"	inline void operator ++() { this->val++; }" "\n"
"	inline void operator --() { this->val--; }" "\n"

"	inline void operator ++(int) { this->val++; }" "\n"
"	inline void operator --(int) { this->val--; }" "\n"

"};\n" "\n"

"class i32 : public Number<int> {" "\n"
"public:" "\n"
"	using Number::Number;" "\n"
"};\n" "\n"

"class i64 : public Number<long long> {" "\n"
"public:" "\n"
"	using Number::Number;" "\n"
"};\n" "\n"

"class u32 : public Number<unsigned int> {" "\n"
"public:" "\n"
"	using Number::Number;" "\n"
"};\n" "\n"

"class u64 : public Number<unsigned long long> {" "\n"
"public:" "\n"
"	using Number::Number;" "\n"
"};\n" "\n"

"class f32 : public Number<float> {" "\n"
"public:" "\n"
"	using Number::Number;" "\n"
"};\n" "\n"

"class f64 : public Number<double> {" "\n"
"public:" "\n"
"	using Number::Number;" "\n"
"};\n" "\n"

"template <typename T>" "\n"
"static inline void println(T s) { std::cout << T(s).display() << std::endl; }" "\n"
"template <typename T>" "\n"
"static inline void print(T s) { std::cout << T(s).display(); }\n" "\n"

"static inline void println(const char* s) { std::cout << s << std::endl; }" "\n"
"static inline void print(const char* s) { std::cout << s; }\n" "\n"

"template <typename T = str>" "\n"
"T scan() { std::string t; std::getline(std::cin, t); return T(t); }\n" "\n"

"[[noreturn]] void panic(str msg) { std::cout << \"\\x1b[31m\" << \"[PANIC] \" + msg + \"\\x1b[0m\" << std::endl; exit(101); }\n" "\n";

	if (has_vecs) {
		LOG_IF_V("Creating Vec header");
		output << vec_impl << '\n';
	}

	bool in_string = false;
	unsigned int current_line = 0;

	std::regex index_for = std::regex("^\\s*for +\\((\\S+) +(\\S+) +in +(\\S+) *\\.{2} *(\\S+) *\\) *(\\{)?");
	std::regex index_for_inclusive = std::regex("^\\s*for +\\((\\S+) +(\\S+) +in +(\\S+) *\\.{2}= *(\\S+) *\\) *(\\{)?");
	std::regex range_for = std::regex("^\\s*for +\\((\\S+) +(\\S+) +in +(\\S+) *\\) *(\\{)?");

	std::regex int_main = std::regex("^\\s*i32\\s+main\\(\\s*\\)\\s*\\{?[^\\n\\r]*");
	std::regex int_args_main = std::regex("^\\s*i32\\s+main\\(\\s*Vec\\s*<\\s*str\\s*>\\s*(\\S+)\\)\\s*\\{?[^\\n\\r]*");

	LOG_IF_V("Translating file");

	char c;
	while (file >> c) {

		if (c == '\r') {
			continue;
		} else if (c == '\n') {
			current_line++;
		}

		if (c == '"' && (ss.str().back() != '\\' || ss.str().size() == 0)) {
			in_string = !in_string;
		}

		if (!in_string && c == '\t') {
			output << "\t";
			continue;
		}

		if (!in_string && c == '\n') {

			if (std::string n = std::regex_replace(ss.str(), int_args_main, "int main(int argc, char** argv) {\n\n\tVec<str> $1;\n\tfor (int i = 0; i < argc; i++) {\n\t\t$1 << argv[i];\n\t}\n"); n != ss.str()) {
				output << n;
			} else if (std::string n = std::regex_replace(ss.str(), int_main, "int main() {\n"); n != ss.str()) {
				output << n;
			} else if (std::string n = std::regex_replace(ss.str(), index_for_inclusive, "for ($1 $2 = $3; $2 <= $4; $2++) $5\n"); n != ss.str()) {
				output << n;
			} else if (std::string n = std::regex_replace(ss.str(), index_for, "for ($1 $2 = $3; $2 < $4; $2++) $5\n"); n != ss.str()) {
				output << n;
			} else if (std::string n = std::regex_replace(ss.str(), range_for, "for ($1 $2 : $3) $4\n"); n != ss.str()) {
				output << n;
			} else {
				std::vector<int> quotes = find_quotes(ss.str());

				bool formatting = false;
				if (quotes.size() != 0) {
					for (int i = 0; i < quotes.size(); i += 2) {
						if (ss.str().substr(quotes[i], quotes[i+1]-quotes[i]+2).find('{') != std::string::npos) {
							formatting = true;
							break;
						}
					}
				}

				if (!formatting) {
					output << ss.str() << "\n";
				} else {
					output << ss.str().substr(0, quotes[0]);
					for (int i = 0; i < quotes.size(); i += 2) {
						std::optional<std::string> format = format_string(ss.str().substr(quotes[i], quotes[i+1]-quotes[i]+2));
						if (format.has_value()) {
							output << format.value() << (i == quotes.size()-1 ? ss.str().substr(quotes[i+1]+1) : ss.str().substr(quotes[i+1]+1, quotes[i+2]-quotes[i+1]-1)) << '\n';
						} else {
							std::cout << RED << '[' << "ERROR" << "] No ending bracket for format string\n " << RESET << "--> " << input_file << ":" << current_line << "\n\t" << ss.str().substr(quotes[i], quotes[i+1]-quotes[i]+1) << std::endl;
							return 3;
						}
					}
				}
			}

			ss.str(std::string());
			continue;
		}

		ss << c;

	}

	file.close();

	output << ss.str();

	std::string name = parser.get<std::string>("--output");

	std::ofstream file_out = std::ofstream(name + ".cpp");
	file_out << output.str();
	file_out.close();

	LOG_IF_V("Spawning g++");

	pid_t pid;
	std::vector<std::string> gcc_args = { "g++", name + ".cpp", "-o", name };
	if (parser["-prod"] == true) {
		LOG_IF_V("Compiling with -O3");
		gcc_args.push_back("-O3");
	}

	if (parser["--static"] == true) {
		LOG_IF_V("Compiling with -static");
		gcc_args.push_back("-static");
	}

	char** gcc_argsc = (char**)malloc(sizeof(char*)*(gcc_args.size()+1));
	for (int i = 0; i < gcc_args.size(); i++) {
		gcc_argsc[i] = (char*)gcc_args[i].c_str();
	}
	gcc_argsc[gcc_args.size()] = NULL;

	int status = posix_spawn(&pid, "/usr/bin/g++", NULL, NULL, gcc_argsc, environ);

	LOG_IF_V("Waiting for g++");
	waitpid(pid, &status, 0);

	free((void*)gcc_argsc);

	std::chrono::duration<double> elapsed = std::chrono::system_clock::now().time_since_epoch() - start;

	if (parser["--keep-cpp"] == false) {
		LOG_IF_V("Deleting " << name + ".cpp");
		std::remove((name + ".cpp").c_str());
	}

	std::cout << GREEN << "[DONE] Compiled in " << round(elapsed.count()*1000) << "ms" << RESET << std::endl;

	return 0;

}
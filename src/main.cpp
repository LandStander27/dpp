#include "defs.hpp"
#include "argparse/argparse.hpp"

extern char **environ;

const std::string vec_impl =
"template<typename T>" "\n"
"class Vec {" "\n"
"	std::vector<T> v;" "\n"
"public:" "\n"
"	Vec(std::initializer_list<T> init) {" "\n"
"		this->v = std::vector<T>(init);" "\n"
"	}" "\n"
"	inline u64 len() {" "\n"
"		return this->v.size();" "\n"
"	}" "\n"
"	void operator<<(T t) {" "\n"
"		this->v.push_back(t);" "\n"
"	}" "\n"
"	typename std::vector<T>::iterator begin() { return this->v.begin(); }" "\n"
"	typename std::vector<T>::iterator end() { return this->v.end(); }" "\n"

"	template<typename U>" "\n"
"	U operator[](U i) {" "\n"
"		if (i >= this->len()) {" "\n"
"			std::cout << RED << \"[RUNTIME] Index out of bounds: len is \" << this->len() << \", index is \" << i << RESET << std::endl;" "\n"
"			exit(101);" "\n"
"		}" "\n"
"		return this->v[i];" "\n"
"	}" "\n"
"};\n" "\n"

"template<typename T>" "\n"
"std::ostream& operator<<(std::ostream& os, Vec<T>& vec) {" "\n"
"	os << \"{ \";" "\n"
"	for (int i = 0; i < vec.len(); i++) {" "\n"
"		os << vec[i];" "\n"
"		if (i < vec.len() - 1) {" "\n"
"			os << \", \";" "\n"
"		}" "\n"
"	}" "\n"
"	os << \" }\";" "\n"
"	return os;" "\n"
"}\n";

#define LOG_IF_V(x) if (parser["--verbose"] == true) { LOG(x); }

std::optional<std::string> format_string(std::string org) {

	std::stringstream ss;

	ss << "({";

	int string_start = org.find("\"");
	ss << "std::stringstream s; ";
	ss << "s << ";
	for (int i = string_start; i < org.size()-2; i++) {
		if (org[i] == '{' && (i != org.size()-3 ? org[i+1] != '{' : true)) {
			int end = org.find('}', i);
			if (end == std::string::npos) {
				return std::nullopt;
			}

			ss << "\" << (" << org.substr(i+1, end-i-1) << ") << \"";
			i = end;
			continue;
		} else if (org[i] == '{') {
			i += 1;
		}
		ss << org[i];
	}
	ss << "\"; s.str();})";

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

"#define RESET \"\\x1b[0m\"" "\n"
"#define RED \"\\x1b[31m\"" "\n"
"#define DARKRED \"\\x1b[91m\"" "\n"
"#define GREEN \"\\x1b[32m\"\n" "\n"

"template <typename T>" "\n"
"class Number {" "\n"
"	T val;\n" "\n"

"public:" "\n"
"	Number(T val) {" "\n"
"		this->val = val;" "\n"
"	}\n" "\n"

"	operator T() {" "\n"
"		return this->val;" "\n"
"	}\n" "\n"

"	T operator -(T other) { return std::move(Number(this->val - other.val)); }" "\n"
"	T operator +(T other) { return std::move(Number(this->val + other.val)); }" "\n"
"	T operator /(T other) { return std::move(Number(this->val / other.val)); }" "\n"
"	T operator *(T other) { return std::move(Number(this->val * other.val)); }\n" "\n"

"	void operator -=(T other) { this->val -= other.val; }" "\n"
"	void operator +=(T other) { this->val += other.val; }" "\n"
"	void operator /=(T other) { this->val /= other.val; }" "\n"
"	void operator *=(T other) { this->val *= other.val; }\n" "\n"

"};\n" "\n"

"class i32 : public Number<int> {" "\n"
"public:" "\n"
"	i32(int val) : Number(val) {}" "\n"
"};\n" "\n"

"class i64 : public Number<long long> {" "\n"
"public:" "\n"
"	i64(long long val) : Number(val) {}" "\n"
"};\n" "\n"

"class u32 : public Number<unsigned int> {" "\n"
"public:" "\n"
"	u32(unsigned int val) : Number(val) {}" "\n"
"};\n" "\n"

"class u64 : public Number<unsigned long long> {" "\n"
"public:" "\n"
"	u64(unsigned long long val) : Number(val) {}" "\n"
"};\n" "\n"

"class f32 : public Number<float> {" "\n"
"public:" "\n"
"	f32(float val) : Number(val) {}" "\n"
"};\n" "\n"

"class f64 : public Number<double> {" "\n"
"public:" "\n"
"	f64(double val) : Number(val) {}" "\n"
"};\n" "\n"

"typedef std::string str;\n" "\n"

"static inline void println(str s) { std::cout << s << std::endl; }" "\n"
"static inline void print(str s) { std::cout << s; }" "\n"
"static inline void println(str& s) { std::cout << s << std::endl; }" "\n"
"static inline void print(str& s) { std::cout << s; }\n" "\n"

"std::string scan() { std::string t; std::getline(std::cin, t); return std::move(t); }" "\n";

	if (has_vecs) {
		LOG_IF_V("Creating Vec header");
		output << vec_impl << '\n';
	}

	bool in_string = false;
	unsigned int current_line = 0;

	std::regex index_for = std::regex("\\s*for +\\((\\S+) +(\\S+) +in +(\\S+) *\\.{2} *(\\S+) *\\) *(\\{)?");
	std::regex range_for = std::regex("\\s*for +\\((\\S+) +(\\S+) +in +(\\S+) *\\) *(\\{)?");

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

			if (std::string n = std::regex_replace(ss.str(), index_for, "for ($1 $2 = $3; $2 < $4; $2++) $5\n"); n != ss.str()) {
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
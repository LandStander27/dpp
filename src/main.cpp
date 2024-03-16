#include "defs.hpp"

extern char **environ;

const std::string vec_impl =
"template<typename T>" "\n"
"class Vec {" "\n"
"	std::vector<T> v;" "\n"
"public:" "\n"
"	Vec(std::initializer_list<T> init) {" "\n"
"		this->v = std::vector<T>(init);" "\n"
"	}" "\n"
"	u64 len() {" "\n"
"		return this->v.size();" "\n"
"	}" "\n"
"	void operator<<(T& t) {" "\n"
"		this->v.push_back(t);" "\n"
"	}" "\n"
"	void operator<<(T t) {" "\n"
"		this->v.push_back(t);" "\n"
"	}" "\n"
"	T operator[](u64 i) {" "\n"
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

	if (argc < 2) {
		std::cout << "No input file" << std::endl;
		return 1;
	}

	std::string input_file = "";
	input_file = std::string(argv[1]);

	if (!std::filesystem::exists(std::filesystem::path(input_file))) {
		std::cout << "Input file does not exist" << std::endl;
		return 2;
	}

	std::ifstream file = std::ifstream(input_file);

	// unsigned int size = std::filesystem::file_size(input_file);
	// std::string input = std::string(size, '\0');
	// file.read(&input[0], size);

	std::stringstream ss;
	file >> std::noskipws;

	bool has_vecs = false;

	int size = std::filesystem::file_size(input_file);
	std::string* entire_file = new std::string(size, '\0');
	file.read(entire_file->data(), size);
	file.seekg(0, file.beg);

	if (entire_file->find("Vec") != std::string::npos) {
		has_vecs = true;
	}

	free(entire_file);

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

"typedef int i32;" "\n"
"typedef long long i64;" "\n"
"typedef unsigned int u32;" "\n"
"typedef unsigned long long u64;" "\n"
"typedef float f32;" "\n"
"typedef double f64;" "\n"
"typedef std::string str;\n" "\n"

"void println(str s) { std::cout << s << std::endl; }" "\n"
"void print(str s) { std::cout << s; }" "\n"
"void println(str& s) { std::cout << s << std::endl; }" "\n"
"void print(str& s) { std::cout << s; }\n" "\n";

	if (has_vecs) {
		output << vec_impl << '\n';
	}

	bool in_string = false;
	unsigned int current_line = 0;

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

			ss.str(std::string());
			continue;
		}

		ss << c;

	}

	output << ss.str();

	std::cout << output.str() << std::endl;

	std::string name = input_file.find_last_of(".") != std::string::npos ? input_file.substr(0, input_file.find_last_of(".")) : input_file;

	std::ofstream file_out = std::ofstream(name + ".cpp");
	file_out << output.str();
	file_out.close();

	pid_t pid;
	std::vector<std::string> gcc_args = { "g++", name + ".cpp", "-o", name };

	char** gcc_argsc = (char**)malloc(sizeof(char*)*(gcc_args.size()+1));
	for (int i = 0; i < gcc_args.size(); i++) {
		gcc_argsc[i] = (char*)gcc_args[i].c_str();
	}
	gcc_argsc[gcc_args.size()] = NULL;

	int status = posix_spawn(&pid, "/usr/bin/g++", NULL, NULL, gcc_argsc, environ);
	waitpid(pid, &status, 0);

	free((void*)gcc_argsc);

	// std::remove((name + ".cpp").c_str());

	file.close();

	return 0;

}
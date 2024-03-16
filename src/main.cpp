#include "defs.hpp"

extern char **environ;

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

			ss << "\" << " << org.substr(i+1, end-i-1) << " << \"";
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

	std::stringstream output;
	output << "#include <iostream>\n";
	output << "#include <sstream>\n\n";

	output << "typedef int i32;\n";
	output << "typedef long long i64;\n";
	output << "typedef unsigned int u32;\n";
	output << "typedef unsigned long long u64;\n";
	output << "typedef float f32;\n";
	output << "typedef double f64;\n";
	output << "typedef std::string str;\n\n";

	output << "void println(str s) {\n\tstd::cout << s << std::endl;\n}\n\n";
	output << "void print(str s) {\n\tstd::cout << s;\n}\n\n";
	output << "void println(str& s) {\n\tstd::cout << s << std::endl;\n}\n\n";
	output << "void print(str& s) {\n\tstd::cout << s;\n}\n\n";

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

	std::remove((name + ".cpp").c_str());

	file.close();

	return 0;

}
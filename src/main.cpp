#include "defs.hpp"

extern char **environ;

std::string format_println(std::string org) {

	std::stringstream ss;
	ss << "std::cout << ";

	int string_start = org.find("(")+1;
	// for (int i = 0; i < org.size(); i++) {
	// 	if (org[i] == '(') {
	// 		string_start = i+1;
	// 		break;
	// 	}
	// }

	for (int i = string_start; i < org.size()-3; i++) {
		if (org[i] == '{') {
			int end = org.find('}', i);
			ss << "\" << " << org.substr(i+1, end-i-1) << " << \"";
			i = end;
			continue;
		}
		ss << org[i];
	}

	ss << " << std::endl;";

	return ss.str();

}

std::string format_print(std::string org) {

	std::stringstream ss;
	ss << "std::cout << ";

	int string_start = org.find("(")+1;
	// for (int i = 0; i < org.size(); i++) {
	// 	if (org[i] == '(') {
	// 		string_start = i+1;
	// 		break;
	// 	}
	// }

	for (int i = string_start; i < org.size()-3; i++) {
		if (org[i] == '{') {
			int end = org.find('}', i);
			ss << "\" << " << org.substr(i+1, end-i-1) << " << \"";
			i = end;
			continue;
		}
		ss << org[i];
	}

	ss << ";";

	return ss.str();

}

std::string format_string_def(std::string org) {

	std::stringstream ss;

	std::string name = org.substr(4, org.find("=")-5);
	ss << "str " << name << ";\n";

	ss << "{\n";

	int string_start = org.find("\"");
	ss << "std::stringstream s;\n";
	ss << "s << ";
	for (int i = string_start; i < org.size()-2; i++) {
		if (org[i] == '{') {
			int end = org.find('}', i);
			ss << "\" << " << org.substr(i+1, end-i-1) << " << \"";
			i = end;
			continue;
		}
		ss << org[i];
	}
	ss << ";\n" << name << " = s.str();\n}";

	return ss.str();

}

std::string format_string(std::string org) {

	std::stringstream ss;

	ss << "({";

	int string_start = org.find("\"");
	ss << "std::stringstream s; ";
	ss << "s << ";
	for (int i = string_start; i < org.size()-2; i++) {
		if (org[i] == '{') {
			int end = org.find('}', i);
			ss << "\" << " << org.substr(i+1, end-i-1) << " << \"";
			i = end;
			continue;
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

	output << "void println(str s) {\n\tstd::cout << s << std::endl;\n}\n";
	output << "void print(str s) {\n\tstd::cout << s;\n}\n";

	bool in_string = false;

	char c;
	while (file >> c) {

		if (c == '"' && (ss.str().back() != '\\' || ss.str().size() == 0)) {
			in_string = !in_string;
		}

		if (!in_string && c == '\t') {
			output << "\t";
			continue;
		}

		if (!in_string && c == '\n') {

			std::vector<int> quotes = find_quotes(ss.str());
			if (quotes.size() == 0) {
				output << ss.str() << "\n";
			} else {
				output << ss.str().substr(0, quotes[0]);
				for (int i = 0; i < quotes.size(); i += 2) {
					output << format_string(ss.str().substr(quotes[i], quotes[i+1]-quotes[i]+2)) << (i == quotes.size()-1 ? ss.str().substr(quotes[i+1]+1) : ss.str().substr(quotes[i+1]+1, quotes[i+2]-quotes[i+1]-1));
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
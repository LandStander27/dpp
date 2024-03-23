#include "defs.hpp"
#include "argparse/argparse.hpp"

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
"		for (u64 i = 0; i < len; i++) {" "\n"
"			this->v.push_back(val);" "\n"
"		}" "\n"
"	}" "\n"
"	inline u64 len() {" "\n"
"		return this->v.size();" "\n"
"	}" "\n"
"	std::string display() {" "\n"
"		std::stringstream os;" "\n"
"		os << \"{ \";" "\n"
"		for (u64 i = 0; i < this->len(); i++) {" "\n"
"			os << (*this)[i].display();" "\n"
"			if (i < this->len() - 1) {" "\n"
"				os << \", \";" "\n"
"			}" "\n"
"		}" "\n"
"		os << \" }\";" "\n"
"		return os.str();" "\n"
"	}" "\n"
"	bool contains(T val) {" "\n"
"		for (u64 i = 0; i < this->len(); i++) {" "\n"
"			if (this->v[i] == val) {" "\n"
"				return true;" "\n"
"			}" "\n"
"		}" "\n"
"		return false;" "\n"
"	}" "\n"
"	void operator<<(T t) {" "\n"
"		this->v.push_back(t);" "\n"
"	}" "\n"

"	T& operator[](u64 i) {" "\n"
"		if (i >= this->len()) {" "\n"
"			panic(\"Index out of bounds: len is \" + this->len().to_string() + \", index is \" + i.to_string());" "\n"
"		}" "\n"
"		return this->v[i.data()];" "\n"
"	}\n" "\n"

"	T* begin() { return &(this->v[0]); }" "\n"
"	T* end() { return &(this->v[this->len().data()]); }" "\n"
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

std::string spawn(std::string cmd) {
	std::array<char, 128> buffer;
	std::string result;
	std::shared_ptr<FILE> pipe(popen((cmd + " 2>&1").c_str(), "r"), pclose);
	if (!pipe) {
		throw std::runtime_error("popen() failed!");
	}
	while (!feof(pipe.get())) {
		if (fgets(buffer.data(), 128, pipe.get()) != nullptr) {
			result += buffer.data();
		}
	}
	return result;
}

std::tuple<std::string, std::vector<std::vector<unsigned long long>>> translate(std::string input_file, argparse::ArgumentParser& parser, bool is_topmost) {
	LOG_IF_V("Opening file " << input_file);

	std::ifstream file = std::ifstream(input_file);

	// unsigned int size = std::filesystem::file_size(input_file);
	// std::string input = std::string(size, '\0');
	// file.read(&input[0], size);

	std::stringstream ss;
	file >> std::noskipws;

	bool has_vecs = false;

	LOG_IF_V("Reading file " << input_file);

	int size = std::filesystem::file_size(input_file);
	std::string* entire_file = new std::string(size, '\0');
	file.read(entire_file->data(), size);
	file.close();
	file.open(input_file);

	std::stringstream output;

	if (is_topmost) {
		LOG_IF_V("Testing Vec existence");

		if (entire_file->find("Vec") != std::string::npos) {
			has_vecs = true;
		}

		LOG_IF_V("Creating header");

		if (has_vecs) {
			output << "#include <vector>" "\n";
		}
		output << default_header;

		if (has_vecs) {
			LOG_IF_V("Creating Vec header");
			output << vec_impl << '\n';
		}
	}

	free(entire_file);

	bool in_string = false;
	unsigned int current_line = 0;

	std::regex index_for = std::regex("^\\s*for +\\((\\S+) +(\\S+) +in +(\\S+) *\\.{2} *(\\S+) *\\) *(\\{)?");
	std::regex index_for_inclusive = std::regex("^\\s*for +\\((\\S+) +(\\S+) +in +(\\S+) *\\.{2}= *(\\S+) *\\) *(\\{)?");
	std::regex range_for = std::regex("^\\s*for +\\((\\S+) +(\\S+) +in +(\\S+) *\\) *(\\{)?");

	std::regex int_main = std::regex("^\\s*i32\\s+main\\(\\s*\\)\\s*\\{?[^\\n\\r]*");
	std::regex int_args_main = std::regex("^\\s*i32\\s+main\\(\\s*Vec\\s*<\\s*str\\s*>\\s*(\\S+)\\)\\s*\\{?[^\\n\\r]*");

	std::regex panic_call = std::regex("^\\s*\\s*panic\\s*\\(\\s*\"(.*)\"\\s*\\)\\s*;");

	std::regex cpp_include = std::regex("^\\s*#cpp-include\\s+(<|\")(.+)(\"|>)");
	std::regex normal_include = std::regex("^\\s*#include\\s+(<|\")(.+)(\"|>)");

	LOG_IF_V("Translating file");

	std::vector<std::vector<unsigned long long>> lines;

	char c;
	while (file >> c) {
		if (c == '\r') {
			continue;
		} else if (c == '\n') {
			unsigned long long output_line = ({std::string s = output.str(); std::count(s.begin(), s.end(), '\n')+1;});
			current_line++;
			lines.push_back({ current_line, output_line });
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
			} else if (std::string n = std::regex_replace(ss.str(), panic_call, "panic(\"$1\", " + std::to_string(current_line) + ", \"" + std::filesystem::path(input_file).filename().string() + "\");\n"); n != ss.str()) {
				output << n;
			} else if (std::string n = std::regex_replace(ss.str(), cpp_include, "#include $1$2$3\n"); n != ss.str()) {
				output << n;
			} else if (std::string n = std::regex_replace(ss.str(), normal_include, "$2"); n != ss.str()) {
				#ifdef _WIN32
				for (const auto& entry : std::filesystem::directory_iterator(get_binary_path() + "\\include")) {
				#else
				for (const auto& entry : std::filesystem::directory_iterator(get_binary_path() + "/include")) {
				#endif
					if (entry.path().filename() == n) {
						auto [translated, other_lines] = translate(entry.path().string(), parser, false);
						if (translated == "err\r") {
							return { translated, other_lines };
						}
						output << translated << '\n';
						break;
					}
				}
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
							std::cout << RED << '[' << "ERROR" << "] No ending bracket for format string\n\t" << "--> " << input_file << ":" << current_line << "\n\t" << ss.str().substr(quotes[i], quotes[i+1]-quotes[i]+1) << RESET << std::endl;
							return std::tuple("err\r", lines);
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
	lines.push_back({ current_line, (unsigned long long)({std::string s = output.str(); std::count(s.begin(), s.end(), '\n')+1;}) });
	return std::tuple(output.str(), lines);
}

int main(int argc, char** argv) {

	argparse::ArgumentParser parser = argparse::ArgumentParser("D++ compiler");

	parser.add_argument("input_file").help("Input file").required().default_value(std::string("./main.dpp"));
	parser.add_argument("-o", "--output").help("Output file").default_value(std::string("a.out"));
	parser.add_argument("-kc", "--keep-cpp").help("Keep the translated C++").default_value(false).implicit_value(true);
	parser.add_argument("-v", "--verbose").help("Verbose output").default_value(false).implicit_value(true);
	parser.add_argument("-prod").help("Heavy optimization").default_value(false).implicit_value(true);
	parser.add_argument("-s", "--static").help("Static linking").default_value(false).implicit_value(true);
	parser.add_argument("-r", "--run").help("Run the compiled program").default_value(false).implicit_value(true);
	parser.add_argument("-l", "--lib").help("Add a library").default_value<std::vector<std::string>>({}).append();
	parser.add_argument("--graphics").help("Link graphics dll (use this if you are using the built in \"graphics.dpp\")").default_value(false).implicit_value(true);

	try {
		parser.parse_args(argc, argv);
	} catch (const std::exception& err) {
		std::cout << err.what() << std::endl;
		std::cout << parser;
		return 1;
	}

	#ifndef _WIN32
	if (parser["--graphics"] == true) {
		std::cout << "Graphics library is only supported on Windows for now" << std::endl;
		return 4;
	}
	#endif

	std::string input_file = parser.get<std::string>("input_file");

	if (!std::filesystem::exists(input_file)) {
		std::cout << "Input file does not exist" << std::endl;
		return 2;
	}

	if (!std::filesystem::is_regular_file(input_file)) {
		std::cout << "Invalid file" << std::endl;
		return 2;
	}

	std::chrono::duration<double> start = std::chrono::system_clock::now().time_since_epoch();

	LOG("Compiling " << input_file);

	std::string name = parser.get<std::string>("--output");

	std::ofstream file_out = std::ofstream(name + ".cpp");
	auto [output, lines] = translate(input_file, parser, true);
	if (output == "err\r") {
		return -1;
	}
	file_out << output;
	file_out.close();

	LOG_IF_V("Spawning g++");

	// pid_t pid;
	// std::vector<std::string> gcc_args = { "g++", name + ".cpp", "-o", name };
	// if (parser["-prod"] == true) {
	// 	LOG_IF_V("Compiling with -O3");
	// 	gcc_args.push_back("-O3");
	// }

	// if (parser["--static"] == true) {
	// 	LOG_IF_V("Compiling with -static");
	// 	gcc_args.push_back("-static");
	// }

	// char** gcc_argsc = (char**)malloc(sizeof(char*)*(gcc_args.size()+1));
	// for (int i = 0; i < gcc_args.size(); i++) {
	// 	gcc_argsc[i] = (char*)gcc_args[i].c_str();
	// }
	// gcc_argsc[gcc_args.size()] = NULL;

	// int status = posix_spawn(&pid, "/usr/bin/g++", NULL, NULL, gcc_argsc, environ);

	// LOG_IF_V("Waiting for g++");
	// waitpid(pid, &status, 0);

	// free((void*)gcc_argsc);

	#ifdef _WIN32
	std::string gcc_args = "g++.exe " + name + ".cpp -o " + name + " -L" + get_binary_path() + "\\include\\lib_deps -I" + get_binary_path() + "\\include";
	#else
	std::string gcc_args = "g++ " + name + ".cpp -o " + name + " -I" + get_binary_path() + "/include";
	#endif
	if (parser["-prod"] == true) {
		LOG_IF_V("Compiling with -O3");
		gcc_args += " -O3 ";
	}

	if (parser["--static"] == true) {
		LOG_IF_V("Compiling with -static");
		gcc_args += " -static ";
	}

	if (parser["--graphics"] == true) {
		LOG_IF_V("Compiling with " + get_binary_path() + "\\include\\lib_deps\\libglfw3.a");
		gcc_args += " " + get_binary_path() + "\\include\\lib_deps\\libglfw3.a ";
		LOG_IF_V("Compiling with -lgdi32");
		gcc_args += " -lgdi32 ";
	}

	for (const auto& lib : parser.get<std::vector<std::string>>("--lib")) {
		gcc_args += " -l" + lib + " ";
	}

	LOG_IF_V(gcc_args);
	std::string gcc_output = spawn(gcc_args);

	std::regex error_regex = std::regex("(.+(\\/|\\\\)(.+))\\.(c|h)(pp)?:([0-9]+):[0-9]+: error: (.+)");
	std::regex fatal_error_regex = std::regex("(.+(\\/|\\\\)(.+))\\.(c|h)(pp)?:([0-9]+):[0-9]+: fatal error: (.+)");

	std::smatch m;
	bool err = false;
	if (std::regex_search(gcc_output, m, error_regex)) {
		err = true;
		int line = std::stoi(m[6]);
		bool found = false;
		for (const auto& l : lines) {
			if (l[1] == line) {
				std::string filename = (m[1] == name) ? input_file : (m[1].str() + "." + m[4].str() + m[5].str());
				std::cout << RED << "[ERROR]: " << m[7];
				if (m[7].str().find("which is of non-class type ‘int’") != std::string::npos) {
					std::cout << ", did you mean i32?";
				} else if (m[7].str().find("which is of non-class type ‘float’") != std::string::npos) {
					std::cout << ", did you mean f32?";
				} else if (m[7].str().find("which is of non-class type ‘double’") != std::string::npos) {
					std::cout << ", did you mean f64?";
				} else if (m[7].str().find("which is of non-class type ‘unsigned int’") != std::string::npos) {
					std::cout << ", did you mean u32?";
				} else if (m[7].str().find("which is of non-class type ‘unsigned long long’") != std::string::npos) {
					std::cout << ", did you mean u64?";
				} else if (m[7].str().find("which is of non-class type ‘long long’") != std::string::npos) {
					std::cout << ", did you mean i64?";
				}
				std::cout << "\n\t--> " << filename << ":" << l[0] << RESET << std::endl;
				found = true;
				break;
			}
		}
		if (!found) {
			std::cout << RED << "[ERROR]: " << m[7] << "\n\t--> " << m[1] << "." << m[4] << m[5] << ":" << m[6] << "\n\tThis error is a problem with the compiler, please run again with --keep-cpp and report the generated C++." << RESET << std::endl;
		}
	} else if (std::regex_search(gcc_output, m, fatal_error_regex)) {
		err = true;
		int line = std::stoi(m[6]);
		bool found = false;
		for (const auto& l : lines) {
			if (l[1] == line) {
				std::string filename = (m[1] == name) ? input_file : (m[1].str() + "." + m[4].str() + m[5].str());
				std::cout << RED << "[ERROR]: " << m[7];
				if (m[7].str().find("which is of non-class type ‘int’") != std::string::npos) {
					std::cout << ", did you mean i32?";
				} else if (m[7].str().find("which is of non-class type ‘float’") != std::string::npos) {
					std::cout << ", did you mean f32?";
				} else if (m[7].str().find("which is of non-class type ‘double’") != std::string::npos) {
					std::cout << ", did you mean f64?";
				} else if (m[7].str().find("which is of non-class type ‘unsigned int’") != std::string::npos) {
					std::cout << ", did you mean u32?";
				} else if (m[7].str().find("which is of non-class type ‘unsigned long long’") != std::string::npos) {
					std::cout << ", did you mean u64?";
				} else if (m[7].str().find("which is of non-class type ‘long long’") != std::string::npos) {
					std::cout << ", did you mean i64?";
				}
				std::cout << "\n\t--> " << filename << ":" << l[0] << RESET << std::endl;
				found = true;
				break;
			}
		}
		if (!found) {
			std::cout << RED << "[ERROR]: " << m[7] << "\n\t--> " << m[1] << "." << m[4] << m[5] << ":" << m[6] << "\n\tThis error is a problem with the compiler, please run again with --keep-cpp and report the generated C++." << RESET << std::endl;
		}
	} else if (gcc_output.length() > 1) {
		std::cout << gcc_output << "\n";
	}

	if (!err) {
		std::chrono::duration<double> elapsed = std::chrono::system_clock::now().time_since_epoch() - start;
		std::cout << GREEN << "[DONE ] Compiled in " << round(elapsed.count()*1000) << "ms" << RESET << std::endl;
	}

	if (parser["--keep-cpp"] == false) {
		LOG_IF_V("Deleting " << name + ".cpp");
		std::remove((name + ".cpp").c_str());
	}

	if (parser["--run"] == true && !err) {
		LOG_IF_V("Running compiled binary");
		#ifndef _WIN32
		int status = system(("./" + name).c_str())/255;
		#else
		int status = system(name.c_str());
		#endif
		if (status != 0) {
			std::cout << RED << "[ERROR] Program exited with status: " << status << RESET << std::endl;
			return status;
		}
	} else {
		return err ? -1 : 0;
	}

}
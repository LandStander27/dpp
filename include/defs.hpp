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

#include <unistd.h>

#ifndef _WIN32

#include <spawn.h>
#include <sys/wait.h>

#else

int posix_spawn(pid_t* pid, const char* binary, void*, void*, char** argv, char** env);
pid_t waitpid(pid_t pid, int* status, int options);

#endif

#define RESET "\x1b[0m"
#define RED "\x1b[31m"
#define DARKRED "\x1b[91m"
#define GREEN "\x1b[32m"

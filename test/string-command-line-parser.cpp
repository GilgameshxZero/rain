// Tests for String::CommandLineParser.
#include <rain/string/command-line-parser.hpp>

#include <cassert>
#include <filesystem>
#include <iostream>

int main() {
	Rain::String::CommandLineParser parser;

	// Synthetic command line. In a real program, these strings are guaranteed to
	// be well-formed.
	char const *argv[] = {
		"--port",
		"80",
		"--live",
		"-Iinclude/",
		"-I../rain/include/",
		"-I",
		"../lib/include/",
		"--include=../ext/lib/include/",
		"--reload",
		"--name=rain"};
	int argc{10};

	unsigned long long port{443};
	bool live{false}, reload{true};
	std::string name{"default"};
	std::vector<std::string> includes;
	parser.addParser("port", port);
	parser.addParser("live", live);
	parser.addParser("reload", reload);
	parser.addParser("name", name);
	parser.addParser("I", includes);
	parser.addParser("include", includes);

	parser.parse(argc, argv);

	std::cout << "Port: " << port << std::endl
						<< "Live: " << live << std::endl
						<< "Reload: " << reload << std::endl
						<< "Name: " << name << std::endl;
	for (std::size_t i{0}; i < includes.size(); i++) {
		std::cout << "Include: " << includes[i] << std::endl;
	}

	assert(port == 80);
	assert(live);
	assert(reload);
	assert(name == "rain");
	assert(includes.size() == 4);

	return 0;
}

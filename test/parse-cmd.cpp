/*
Tests the `CommandLineParser` and `WaterfallParser` string parser classes in
`rain/string/`.
*/

#include <rain/string/command-line-parser.hpp>

#include <cassert>
#include <filesystem>
#include <iostream>

int main() {
	Rain::String::CommandLineParser parser;

	// Synthetic command line.
	char const *argv[] = {"--port",
		"80",
		"--live",
		"-Iinclude/",
		"-I../rain/include/",
		"-I",
		"../lib/include/",
		"--reload",
		"--name=rain"};
	int argc = 9;

	std::size_t port = 443;
	bool live = false, reload = true;
	std::string name = "default";
	std::vector<std::string> includes;
	parser.addLayer("port", &port);
	parser.addLayer("live", &live);
	parser.addLayer("reload", &reload);
	parser.addLayer("name", &name);
	parser.addLayer("I", &includes);

	parser.parse(argc, argv);

	std::cout << "Working directory: " << std::filesystem::current_path()
						<< std::endl
						<< "Port: " << port << std::endl
						<< "Live: " << live << std::endl
						<< "Reload: " << reload << std::endl
						<< "Name: " << name << std::endl;
	for (std::size_t a = 0; a < includes.size(); a++) {
		std::cout << "Include: " << includes[a] << std::endl;
	}

	assert(port == 80);

	return 0;
}

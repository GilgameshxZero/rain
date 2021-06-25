/*
Tests the `CommandLineParser` and `WaterfallParser` string parser classes in
`rain/string/`.
*/

#include <rain/string/command-line-parser.hpp>

#include <filesystem>
#include <iostream>

int main() {
	Rain::String::CommandLineParser parser;
	char const *argv[] = {"--port",
		"80",
		"--live",
		"-Iinclude/",
		"-I../rain/include/",
		"-I",
		"../lib/include/",
		"--reload",
		"--name=GILGAMESH"};
	int argc = 9;

	unsigned long long port = 443;
	bool live = false, reload = true;
	std::string name = "GILGAMESH";
	std::vector<std::string> includes;
	parser.addLayer("port", &port);
	parser.addLayer("live", &live);
	parser.addLayer("reload", &reload);
	parser.addLayer("name", &name);
	parser.addLayer("I", &includes);

	try {
		parser.parse(argc, argv);
	} catch (std::exception const &e) {
		std::cout << e.what() << std::endl;
	}

	std::cout << "Working directory: " << std::filesystem::current_path()
						<< std::endl
						<< "Port: " << port << std::endl
						<< "Live: " << live << std::endl
						<< "Reload: " << reload << std::endl
						<< "Name: " << name << std::endl;
	for (std::size_t a = 0; a < includes.size(); a++) {
		std::cout << "Include: " << includes[a] << std::endl;
	}

	return 0;
}

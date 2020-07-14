#include "rain.hpp"

int main() {
	Rain::String::CommandLineParser parser;
	const char *argv[] = {"--port",
		"80",
		"--live",
		"-Iinclude/",
		"-I../rain/include/",
		"-I",
		"../lib/include/",
		"--reload",
		"--name=Yang"};
	int argc = 9;

	size_t port = 443;
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
	} catch (const std::system_error &err) {
		std::cout << err.what() << std::endl;
	}

	std::cout << "Port: " << port << std::endl
						<< "Live: " << live << std::endl
						<< "Reload: " << reload << std::endl
						<< "Name: " << name << std::endl;
	for (size_t a = 0; a < includes.size(); a++) {
		std::cout << "Include: " << includes[a] << std::endl;
	}

	return 0;
}

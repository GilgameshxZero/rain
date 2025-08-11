// Tests for String::CommandLineParser.
#include <rain.hpp>

int main() {
	Rain::String::CommandLineParser parser;

	// Synthetic command line. In a real program, these strings are guaranteed to
	// be well-formed.
	std::array argv{
		"--port",
		"80",
		"--live",
		"-Iinclude/",
		"-I../rain/include/",
		"-I",
		"../lib/include/",
		"--include=../ext/lib/include/",
		"--reload",
		"--name=rain",
		"filename.txt"};
	std::vector<std::string> nonKeyedArguments;

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

	parser.parse(static_cast<int>(argv.size()), argv.data(), nonKeyedArguments);

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
	assert(nonKeyedArguments.size() == 1);
	assert(nonKeyedArguments.front() == "filename.txt");

	return 0;
}

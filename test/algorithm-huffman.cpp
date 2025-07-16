// Tests for Huffman en/de-coding.
#include <rain/algorithm/huffman.hpp>
#include <rain/filesystem.hpp>
#include <rain/literal.hpp>
#include <rain/time.hpp>

#include <cassert>
#include <fstream>
#include <iostream>

void assertEncodeDecode(std::string const &text) {
	std::ostringstream encodedStream;
	Rain::Algorithm::HuffmanStreamBuf encodeStreamBuf(
		*encodedStream.rdbuf(), text);
	std::ostream encoder(&encodeStreamBuf);
	encoder << text;
	encoder.flush();

	std::string encodedText{encodedStream.str()};

	std::istringstream decodedStream{encodedText};
	Rain::Algorithm::HuffmanStreamBuf decodeStreamBuf(*decodedStream.rdbuf());
	std::istream decoder(&decodeStreamBuf);
	std::string decodedText(
		(std::istreambuf_iterator<char>(decoder)),
		(std::istreambuf_iterator<char>()));

	std::cout << "Text: " << text.substr(0, 16) << "..." << std::endl;
	assert(text == decodedText);
	std::cout << "Compression ratio: "
						<< static_cast<long double>(encodedText.length()) / text.length()
						<< '.' << std::endl
						<< std::endl;
}

void assertEncodeDecodeFile(std::string const &name) {
	using namespace Rain::Literal;

	auto timeBegin = std::chrono::steady_clock::now();
	std::cout << "Filename: " << name << '.' << std::endl;

	std::filesystem::path const filePath{"../test/" + name};
	std::ifstream fileIn(filePath, std::ios::binary);
	std::array<std::size_t, UCHAR_MAX + 1> fileFrequency{0};
	int nextC{fileIn.get()};
	while (fileIn) {
		fileFrequency[nextC]++;
		nextC = fileIn.get();
	}
	fileIn.close();

	std::filesystem::create_directory("algorithm-huffman.cpp-assets");
	std::filesystem::path const encodedPath{name + ".huff"};
	std::ofstream encodedOut(encodedPath, std::ios::binary);
	Rain::Algorithm::HuffmanStreamBuf encodedStreamBuf(
		*encodedOut.rdbuf(), fileFrequency);
	std::ostream encodedStream{&encodedStreamBuf};
	fileIn.open(filePath, std::ios::binary);
	encodedStream << fileIn.rdbuf();
	encodedStream.flush();
	fileIn.close();
	encodedOut.close();

	auto timeMidpoint = std::chrono::steady_clock::now();
	auto encodeTime = timeMidpoint - timeBegin;
	std::cout << "Encode time: " << encodeTime << '.' << std::endl;
	auto originalSize = std::filesystem::file_size(filePath);
	std::cout << "Encode speed: "
						<< static_cast<long double>(originalSize) / (1_zu << 20) /
			std::chrono::duration_cast<std::chrono::duration<long double>>(encodeTime)
				.count()
						<< "MB/s." << std::endl;

	std::filesystem::path const decodedPath{name};
	std::ifstream encodedIn(encodedPath, std::ios::binary);
	std::ofstream decodedOut(decodedPath, std::ios::binary);
	Rain::Algorithm::HuffmanStreamBuf decodedStreamBuf(*encodedIn.rdbuf());
	decodedOut << &decodedStreamBuf;
	decodedOut.close();
	encodedIn.close();

	auto timeEnd = std::chrono::steady_clock::now();
	auto decodeTime = timeEnd - timeMidpoint;
	std::cout << "Decode time: " << decodeTime << '.' << std::endl;
	auto encodedSize = std::filesystem::file_size(encodedPath);
	std::cout << "Decode speed: "
						<< static_cast<long double>(encodedSize) / (1_zu << 20) /
			std::chrono::duration_cast<std::chrono::duration<long double>>(decodeTime)
				.count()
						<< "MB/s." << std::endl;

	assert(Rain::Filesystem::compareFiles(filePath, decodedPath));
	std::cout << "Compression ratio: "
						<< static_cast<long double>(encodedSize) / originalSize << '.'
						<< std::endl
						<< std::endl;
}

int main() {
	using namespace Rain::Literal;

	assertEncodeDecode("Hello world!"s);
	assertEncodeDecode(
		"Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod "
		"tempor incididunt ut labore et dolore magna aliqua. Odio ut enim blandit "
		"volutpat maecenas volutpat blandit. Lacus viverra vitae congue eu "
		"consequat. Sed ullamcorper morbi tincidunt ornare massa eget egestas. "
		"Augue mauris augue neque gravida in. Imperdiet nulla malesuada "
		"pellentesque elit eget gravida. Mattis nunc sed blandit libero volutpat "
		"sed cras. Id neque aliquam vestibulum morbi blandit cursus. Ultricies "
		"lacus sed turpis tincidunt id aliquet risus. Aenean et tortor at risus "
		"viverra adipiscing at. Sodales neque sodales ut etiam sit. Sem nulla "
		"pharetra diam sit amet nisl suscipit. Tincidunt lobortis feugiat vivamus "
		"at augue. Dolor sit amet consectetur adipiscing elit duis tristique. "
		"Sapien et ligula ullamcorper malesuada. Tortor pretium viverra "
		"suspendisse potenti nullam. Ultricies mi eget mauris pharetra "
		"et.\r\n\r\n");
	assertEncodeDecode(
		"aaaaabcbaaaaaaaaaaaaaaaaaaacbbbbbbbbbbbbbcbcbaaaaaaaaabcbacbbbca"s);
	assertEncodeDecode("testing the EOF SUB \x1a and NULL \x00 characters"s);

	assertEncodeDecodeFile("algorithm-huffman.cpp-assets/text.txt");
	assertEncodeDecodeFile("algorithm-huffman.cpp-assets/blue-orb.png");
	assertEncodeDecodeFile("algorithm-huffman.cpp-assets/1905.02175.pdf");

	return 0;
}

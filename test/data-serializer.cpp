// Tests for pickle serialization/deserialization.
#include <rain/data/serializer.hpp>
#include <rain/literal.hpp>

#include <cassert>
#include <fstream>
#include <iostream>
#include <vector>

int main() {
	using namespace Rain::Literal;

	{
		int data[]{1, 4, 5, 10, -5, -49, 3049};
		int *pData = data + 3;
		int **pPData = &pData;
		std::ofstream out("data-serializer.txt", std::ios::binary);
		Rain::Data::Serializer serializer(out);
		serializer << data << pData << pPData;
		out.close();
		decltype(data) deData;
		decltype(pData) dePData;
		decltype(pPData) dePPData;
		std::ifstream in("data-serializer.txt", std::ios::binary);
		Rain::Data::Deserializer deserializer(in);
		deserializer >> deData >> dePData >> dePPData;
		in.close();
		assert(memcmp(&data, &deData, sizeof(data)) == 0);
		assert(memcmp(pData, dePData, sizeof(*pData)) == 0);
		assert(memcmp(*pPData, *dePPData, sizeof(**pPData)) == 0);
	}

	{
		int x = 66;
		int *pX = &x;
		std::ofstream out("data-serializer.txt", std::ios::binary);
		Rain::Data::Serializer serializer(out);
		serializer << pX;
		out.close();
		decltype(pX) dePX = new int;
		std::ifstream in("data-serializer.txt", std::ios::binary);
		Rain::Data::Deserializer deserializer(in);
		deserializer >> dePX;
		in.close();
		assert(memcmp(pX, dePX, sizeof(*pX)) == 0);
		delete dePX;
	}

	{
		int x = 97;
		int *pX = &x;
		std::ofstream out("data-serializer.txt", std::ios::binary);
		Rain::Data::Serializer serializer(out);
		serializer << x << pX;
		out.close();
		decltype(x) deX;
		decltype(pX) dePX;
		std::ifstream in("data-serializer.txt", std::ios::binary);
		Rain::Data::Deserializer deserializer(in);
		deserializer >> deX >> dePX;
		in.close();
		assert(memcmp(&x, &deX, sizeof(x)) == 0);
		assert(memcmp(pX, dePX, sizeof(*pX)) == 0);
	}

	{
		std::vector<long long> a{40586, 2000394953412123LL, 29348, 4059586};
		std::ofstream out("data-serializer.txt", std::ios::binary);
		Rain::Data::Serializer serializer(out);
		serializer << a;
		out.close();
		decltype(a) deA;
		std::ifstream in("data-serializer.txt", std::ios::binary);
		Rain::Data::Deserializer deserializer(in);
		deserializer >> deA;
		in.close();
		assert(a == deA);
	}

	{
		std::string s{"hello world!!! ^_^ :D"};
		std::ofstream out("data-serializer.txt", std::ios::binary);
		Rain::Data::Serializer serializer(out);
		serializer << s;
		out.close();
		decltype(s) deS;
		std::ifstream in("data-serializer.txt", std::ios::binary);
		Rain::Data::Deserializer deserializer(in);
		deserializer >> deS;
		in.close();
		assert(s == deS);
	}

	return 0;
}

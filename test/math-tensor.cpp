#include <rain.hpp>

int main() {
	using namespace Rain::Math;
	using namespace Rain::Random;
	using namespace Rain::String;
	using namespace Rain::Literal;
	using namespace std;

	uniform_int_distribution dist(0, 127);

	{
		// Clean, indexing, data.
		Tensor<int, 2> a{{12, 12}};
		a.applyOver<0>([&dist](int &value) { value = dist(generator); });
		assert(a.isClean());
		assert((a.size() == array<size_t, 2>{{12, 12}}));

		auto b{a.asSlice({{{.step = 2}, {.step = 2}}})};
		assert((b.size() == array<size_t, 2>{{6, 6}}));
		assert(a.data() == b.data());
		assert(b[3][4] == a[6][8]);
		b[1][1] += 300;
		assert(!b.isClean());
		assert(a[2][2] >= 300);
		b.clean();
		assert(b.isClean());
		assert(a.data() != b.data());
		b[1][2] += 300;
		assert(a[2][4] < 300);
		assert((b.size() == array<size_t, 2>{{6, 6}}));
		assert((a.size() == array<size_t, 2>{{12, 12}}));
		cout << a << '\n';
		cout << b << '\n';

		// Reshape.
		auto c{a.asReshape<3>({3, 2, 0})};
		assert((c.size() == array<size_t, 3>{{3, 2, 24}}));
		cout << c << '\n';

		auto d{a.asSlice({{{.step = 3}, {.step = 4}}})};
		assert(!d.isClean());
		cout << d << '\n';
		auto e{d.asReshape<1>({0})};
		assert(e.isClean());
		assert((e.size() == array<size_t, 1>{{12}}));
		e.slice({{{.start = 3}}});
		assert((e.size() == array<size_t, 1>{{9}}));
		cout << e << '\n';

		// Advanced slicing.
		auto f{a.asReshape<1>({0}).slice({{{.start = 19, .stop = 87, .step = 3}}})};
		cout << f << '\n';
		assert(f.size()[0] == 23);
		assert(f[0] == a[1][7]);
		assert(f[1] == a[1][10]);
		assert(f[2] == a[2][1]);
	}

	{
		// Normal product, equality.
		Tensor<int, 2> a{{360, 500}}, b{{500, 420}}, c, d;
		a.applyOver<0>([&dist](int &value) { value = dist(generator); });
		b.applyOver<0>([&dist](int &value) { value = dist(generator); });
		{
			auto timeBegin = std::chrono::steady_clock::now();
			c = a * b;
			auto timeEnd = std::chrono::steady_clock::now();
			std::cout << "Time elapsed (naive): " << timeEnd - timeBegin << '.'
								<< std::endl;
			assert(timeEnd - timeBegin <= 20s);
		}
		{
			auto timeBegin = std::chrono::steady_clock::now();
			d = a.productStrassen(b);
			auto timeEnd = std::chrono::steady_clock::now();
			std::cout << "Time elapsed (Strassen): " << timeEnd - timeBegin << '.'
								<< std::endl;
			assert(timeEnd - timeBegin <= 20s);
		}
		assert(c == d);
		assert((c.size() == array<size_t, 2>{{360, 420}}));

		// Inner/outer.
		Tensor<int, 1> e{{6}, 1, 2, 3, 4, 5, 6}, f{{6}, 7, 8, 9, 10, 11, 12};
		cout << e << '\n' << f << '\n';
		auto g{e.productOuter(f)};
		auto h{e.productInner(f)};
		cout << g << '\n' << h << '\n';
		assert((g == Tensor<int, 2>{{6, 6}, 7,	8,	9,	10, 11, 12, 14, 16, 18,
																20,			22, 24, 21, 24, 27, 30, 33, 36, 28,
																32,			36, 40, 44, 48, 35, 40, 45, 50, 55,
																60,			42, 48, 54, 60, 66, 72}));
		assert((h == 217));

		// Product policy.
		auto i{e.product<1, Tensor<>::MinPlusProductPolicy>(f, {0}, {0})};
		cout << i << '\n';
		assert(i == 8);
	}

	{
		// Stream out.
		Tensor<int, 2> a{{0, 3}}, b{{3, 0}};
		cout << a << '\n' << b << '\n';
		auto c{anyToString(a)}, d{anyToString(b)};
		assert(c == "[]");
		assert(d == "[[]\n []\n []]");

		// Transpose.
		Tensor<int, 3> e{{2, 2, 2}};
		for (int i{0}; i < 2; i++) {
			for (int j{0}; j < 2; j++) {
				for (int k{0}; k < 2; k++) {
					e[i][j][k] = i * 4 + j * 2 + k;
				}
			}
		}
		cout << e << '\n';
		e.transpose({2, 1, 0});
		cout << e << '\n';
		assert((e == Tensor<int, 3>{{2, 2, 2}, 0, 4, 2, 6, 1, 5, 3, 7}));
		e.transpose({2, 1, 0});
		cout << e << '\n';
		assert((e == Tensor<int, 3>{{2, 2, 2}, 0, 1, 2, 3, 4, 5, 6, 7}));
		e.transpose({1, 2, 0});
		cout << e << '\n';
		assert((e == Tensor<int, 3>{{2, 2, 2}, 0, 4, 1, 5, 2, 6, 3, 7}));
	}

	return 0;
}

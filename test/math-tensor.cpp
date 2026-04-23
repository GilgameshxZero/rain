#include <rain.hpp>

using Rain::Error::releaseAssert;

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
		a.applyOver<0>(
			[&dist](int &value) { value = dist(generator); });
		releaseAssert(a.isClean());
		releaseAssert((a.size() == array<size_t, 2>{{12, 12}}));

		auto b{a.asSlice({{{.step = 2}, {.step = 2}}})};
		releaseAssert((b.size() == array<size_t, 2>{{6, 6}}));
		releaseAssert(a.data() == b.data());
		releaseAssert(b[3][4] == a[6][8]);
		b[1][1] += 300;
		releaseAssert(!b.isClean());
		releaseAssert(a[2][2] >= 300);
		b.clean();
		releaseAssert(b.isClean());
		releaseAssert(a.data() != b.data());
		b[1][2] += 300;
		releaseAssert(a[2][4] < 300);
		releaseAssert((b.size() == array<size_t, 2>{{6, 6}}));
		releaseAssert((a.size() == array<size_t, 2>{{12, 12}}));
		cout << a << '\n';
		cout << b << '\n';

		// Reshape.
		auto c{a.asReshape<3>({3, 2, 0})};
		releaseAssert(
			(c.size() == array<size_t, 3>{{3, 2, 24}}));
		cout << c << '\n';

		auto d{a.asSlice({{{.step = 3}, {.step = 4}}})};
		releaseAssert(!d.isClean());
		cout << d << '\n';
		auto e{d.asReshape<1>({0})};
		releaseAssert(e.isClean());
		releaseAssert((e.size() == array<size_t, 1>{{12}}));
		e.slice({{{.start = 3}}});
		releaseAssert((e.size() == array<size_t, 1>{{9}}));
		cout << e << '\n';

		// Advanced slicing.
		auto f{a.asReshape<1>({0}).slice(
			{{{.start = 19, .stop = 87, .step = 3}}})};
		cout << f << '\n';
		releaseAssert(f.size()[0] == 23);
		releaseAssert(f[0] == a[1][7]);
		releaseAssert(f[1] == a[1][10]);
		releaseAssert(f[2] == a[2][1]);
	}

	{
		// Stream out.
		Tensor<int, 2> a{{0, 3}}, b{{3, 0}};
		cout << a << '\n' << b << '\n';
		auto c{anyToString(a)}, d{anyToString(b)};
		releaseAssert(c == "[]");
		releaseAssert(d == "[[]\n []\n []]");

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
		releaseAssert(
			(e ==
			 Tensor<int, 3>{{2, 2, 2}, 0, 4, 2, 6, 1, 5, 3, 7}));
		e.transpose({2, 1, 0});
		cout << e << '\n';
		releaseAssert(
			(e ==
			 Tensor<int, 3>{{2, 2, 2}, 0, 1, 2, 3, 4, 5, 6, 7}));
		e.transpose({1, 2, 0});
		cout << e << '\n';
		releaseAssert(
			(e ==
			 Tensor<int, 3>{{2, 2, 2}, 0, 4, 1, 5, 2, 6, 3, 7}));
		e.swap({0, 1});
		cout << e << '\n';
		releaseAssert(
			(e ==
			 Tensor<int, 3>{{2, 2, 2}, 2, 6, 3, 7, 0, 4, 1, 5}));
		e.transpose({1, 0, 2});
		cout << e << '\n';
		releaseAssert(
			(e ==
			 Tensor<int, 3>{{2, 2, 2}, 2, 6, 0, 4, 3, 7, 1, 5}));
	}

	{
		// Row echelon.
		Tensor<double, 2> a{
			{3, 4},
			1.0,
			2.0,
			0.0,
			0.0,
			2.0,
			4.0,
			1.0,
			7.0,
			1.0,
			4.0,
			1.0,
			2.0};
		cout << a << '\n';
		auto b{a.reduceToRre()};
		cout << b << '\n' << a << '\n';
	}

	{
		// Inner/outer.
		Tensor<int, 1> e{{6}, 1, 2, 3, 4, 5, 6},
			f{{6}, 7, 8, 9, 10, 11, 12};
		cout << e << '\n' << f << '\n';
		auto g{e.productOuter(f)};
		auto h{e.productInner(f)};
		cout << g << '\n' << h << '\n';
		releaseAssert(
			(g == Tensor<int, 2>{{6, 6}, 7,	 8,	 9,	 10, 11, 12,
													 14,		 16, 18, 20, 22, 24, 21,
													 24,		 27, 30, 33, 36, 28, 32,
													 36,		 40, 44, 48, 35, 40, 45,
													 50,		 55, 60, 42, 48, 54, 60,
													 66,		 72}));
		releaseAssert((h == 217));

		// Product policy.
		auto i{e.product<1, Tensor<>::MinPlusProductPolicy>(
			f, {0}, {0})};
		cout << i << '\n';
		releaseAssert(i == 8);

		// Normal product, equality.
		Tensor<int, 2> a{{360, 500}}, b{{500, 420}}, c, d;
		a.applyOver<0>(
			[&dist](int &value) { value = dist(generator); });
		b.applyOver<0>(
			[&dist](int &value) { value = dist(generator); });
		{
			auto timeBegin = std::chrono::steady_clock::now();
			c = a * b;
			auto timeEnd = std::chrono::steady_clock::now();
			std::cout << "Time elapsed (naive): "
								<< timeEnd - timeBegin << '.' << std::endl;
			releaseAssert(timeEnd - timeBegin <= 20s);
		}
		{
			auto timeBegin = std::chrono::steady_clock::now();
			d = a.productStrassen(b);
			auto timeEnd = std::chrono::steady_clock::now();
			std::cout << "Time elapsed (Strassen): "
								<< timeEnd - timeBegin << '.' << std::endl;
			releaseAssert(timeEnd - timeBegin <= 20s);
		}
		releaseAssert(c == d);
		releaseAssert(
			(c.size() == array<size_t, 2>{{360, 420}}));
	}

	return 0;
}

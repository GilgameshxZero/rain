#include <rain.hpp>

using namespace Rain::Math;
using namespace Rain::Random;
using namespace std;

int main() {
	uniform_int_distribution dist(0, 127);
	// Clean and reshape.
	{
		Tensor<int, 2> a{{12, 12}};
		a.applyOver<0>([&dist](int &value) { value = dist(generator); });
		assert(a.isClean());
		// std::array as{12, 13};
		// assert((a.size() == array<int, 2>{13, 14}));

		auto b{a.asSlice({{{.step = 2}, {.step = 2}}})};
		cout << a.size() << '\n' << b.size() << '\n';
		assert(!b.isClean());
		b.clean();
		assert(b.isClean());
		cout << a << '\n';
		cout << b << '\n';
	}

	{
		Tensor<int, 1> a{{12}, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
		cout << a << '\n';
		auto b{a.asReshape<2>({3, 4})};
		cout << b << '\n';
		cout << b.asReshape<3>({2, 0, 2}) << '\n';
		assert(a.data() == b.data());
		auto c{a.asSlice({{{0, 12, 2}}}).asReshape<2>({3, 2})};
		cout << c << '\n';
		assert(a.data() != c.data());
		cout << '\n';
	}

	{
		Tensor<int, 2> a{{10, 10}};
		for (int i{0}; i < 10; i++) {
			for (int j{0}; j < 10; j++) {
				a[i][j] = i * 10 + j;
			}
		}
		assert(a.isClean());
		a = a.slice({{{0, 10, 2}, {0, 10, 2}}});
		assert(!a.isClean());
		cout << a << '\n';
		a.clean();
		assert(a.isClean());
		cout << a << '\n';
		cout << '\n';
	}

	{
		Tensor<int, 2> a{{512, 512}}, b{{512, 512}}, c, d;
		{
			std::cout << "Start set." << std::endl;
			auto timeBegin = std::chrono::steady_clock::now();
			for (int i{0}; i < 512; i++) {
				for (int j{0}; j < 512; j++) {
					a[i][j] = i * 10 + j;
					b[i][j] = 512 * 512 + i * 10 + j;
				}
			}
			auto timeEnd = std::chrono::steady_clock::now();
			std::cout << "Time elapsed (set): " << timeEnd - timeBegin << '.'
								<< std::endl;
		}
		{
			std::cout << "Start multiplication." << std::endl;
			auto timeBegin = std::chrono::steady_clock::now();
			c = a * b;
			auto timeEnd = std::chrono::steady_clock::now();
			std::cout << "Time elapsed (naive): " << timeEnd - timeBegin << '.'
								<< std::endl;
		}
		{
			std::cout << "Start multiplication." << std::endl;
			auto timeBegin = std::chrono::steady_clock::now();
			d = a.productStrassen(b);
			auto timeEnd = std::chrono::steady_clock::now();
			std::cout << "Time elapsed (Strassen): " << timeEnd - timeBegin << '.'
								<< std::endl;
		}
		assert(c == d);
	}

	{
		Tensor<int, 2> a{{0, 3}}, b{{3, 0}};
		cout << a << '\n' << b << '\n';
		cout << '\n';
	}

	{
		Tensor<int, 1> a{{4}, 1, 2, 3, 4}, b{{4}, 5, 6, 7, 8};
		cout << a.productOuter(b) << '\n' << a.productInner(b) << '\n';
		cout << '\n';
	}

	{
		Tensor<int, 2> a{{2, 3}, 1, 2, 3, 4, 5, 6},
			b{{3, 2}, 10, 11, 20, 21, 30, 31};
		auto c{a.product<1>(b, {1}, {0})};
		cout << a << '\n' << b << '\n' << c << '\n';
		auto d{a * b};
		assert(c == d);
		cout << '\n';
	}

	{
		Tensor<int, 2> a{{10, 10}};
		for (int i{0}; i < 10; i++) {
			for (int j{0}; j < 10; j++) {
				a[i][j] = i * 10 + j;
			}
		}
		cout << a << '\n';
		auto b{a.asSlice({{{1, 9, 2}, {2, 10, 3}}})};
		cout << b << '\n';
		b -= 100;
		cout << b << '\n';
		b.transpose({1, 0});
		cout << b << '\n';
		cout << a << '\n';
		cout << '\n';
	}

	{
		Tensor<int, 3> a{{4, 4, 4}};
		for (int i{0}; i < 4; i++) {
			for (int j{0}; j < 4; j++) {
				for (int k{0}; k < 4; k++) {
					a[i][j][k] = i * 16 + j * 4 + k;
				}
			}
		}
		cout << a << '\n';
		a.transpose({2, 1, 0});
		cout << a << '\n';
		a.transpose({2, 1, 0});
		cout << a << '\n';
		a.transpose({1, 2, 0});
		cout << a << '\n';
		cout << '\n';
	}

	{
		Tensor<int, 1> a{{4}, 1, 2, 3};
		a[3] = 100;
		cout << a << '\n';
		Tensor<int, 2> b{{2, 2}, 5, 6, 7, 8};
		b[1][0] = 100;
		cout << b << '\n';
		cout << b.asSlice({{{1, 2, 1}, {0, 2, 1}}}) << '\n';
		cout << b.asSlice({{{1, 2}, {0, 2}}}) << '\n';
		cout << b.asSlice({{{1}, {0}}}) << '\n';

		Tensor<int, 2> c{{10, 10}};
		for (int i{0}; i < 10; i++) {
			for (int j{0}; j < 10; j++) {
				c[i][j] = i * 10 + j;
			}
		}
		cout << c << '\n';
		cout << c.asSlice({{{4}, {4}}}) << '\n';
		cout << c.asSlice({{{.start = 1, .stop = 8, .step = 3}, {.step = 2}}})
				 << '\n';
		cout << c.asSlice({{{4}, {.step = 2}}}) << '\n';
		cout << c.asSlice({{{4}, {.step = 2}}})
							.asSlice({{{}, {.start = 1, .step = 2}}})
				 << '\n';
		cout << c.asSlice({{{4}, {.step = 2}}})
							.asSlice({{{}, {.start = 1, .step = 2}}})[4]
				 << '\n';

		Tensor<int, 3> d{{5, 5, 5}};
		for (int i{0}; i < 5; i++) {
			for (int j{0}; j < 5; j++) {
				for (int k{0}; k < 5; k++) {
					d[i][j][k] = i * 25 + j * 5 + k;
				}
			}
		}
		cout << d << '\n';
		cout << d[3] << '\n' << '\n';
		cout << d.asSlice({{{.step = 2}}}) << "\n\n";
		cout << d.asSlice({{{.step = 2}}}).asSlice({{{}, {.step = 3}}}) << "\n\n";
		cout << d.asSlice({{{.step = 2}}})
							.asSlice({{{}, {.step = 3}}})
							.asSlice({{{}, {}, {.step = 4}}})
				 << "\n\n";

		Tensor<int, 2> e{{2, 2}, 100, 100, 100, 100};
		cout << c.asSlice({{{4, 6}, {4, 6}}}) << "\n\n";
		cout << c.asSlice({{{4, 6}, {4, 6}}}) + e << "\n\n";
		c.asSlice({{{4, 6}, {4, 6}}}) += e;
		cout << c << "\n\n";

		cout << c.asSlice({{{.step = 3}, {.step = 3}}}) << "\n\n";
		Tensor<int, 2> f({4, 4});
		f.fill(999);
		c.asSlice({{{.step = 3}, {.step = 3}}}) -= f;
		cout << c << "\n\n";
		c *= -1;
		cout << c << "\n\n";
		auto g{c.asSlice({{{.step = 4}, {.step = 4}}})};
		cout << g << "\n\n";
		cout << g.asTranspose({{1, 0}}) << "\n\n";
	}
	return 0;
}

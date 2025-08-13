#include <rain.hpp>

using namespace Rain;
using namespace Math;
using namespace std;

int main() {
	{
		Tensor<int, 1> a{{4}, 1, 2, 3};
		a[3] = 100;
		cout << a << '\n';
		Tensor<int, 2> b{{2, 2}, 5, 6, 7, 8};
		b[1][0] = 100;
		cout << b << '\n';
		cout << b.slice({{{1, 2, 1}, {0, 2, 1}}}) << '\n';
		cout << b.slice({{{1, 2}, {0, 2}}}) << '\n';
		cout << b.slice({{{1}, {0}}}) << '\n';

		Tensor<int, 2> c{{10, 10}};
		for (int i{0}; i < 10; i++) {
			for (int j{0}; j < 10; j++) {
				c[i][j] = i * 10 + j;
			}
		}
		cout << c << '\n';
		cout << c.slice({{{4}, {4}}}) << '\n';
		cout << c.slice({{{.start = 1, .stop = 8, .step = 3}, {.step = 2}}})
				 << '\n';
		cout << c.slice({{{4}, {.step = 2}}}) << '\n';
		cout << c.slice({{{4}, {.step = 2}}}).slice({{{}, {.start = 1, .step = 2}}})
				 << '\n';
		cout
			<< c.slice({{{4}, {.step = 2}}}).slice({{{}, {.start = 1, .step = 2}}})[4]
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
		cout << d.slice({{{.step = 2}}}) << "\n\n";
		cout << d.slice({{{.step = 2}}}).slice({{{}, {.step = 3}}}) << "\n\n";
		cout << d.slice({{{.step = 2}}})
							.slice({{{}, {.step = 3}}})
							.slice({{{}, {}, {.step = 4}}})
				 << "\n\n";

		Tensor<int, 2> e{{2, 2}, 100, 100, 100, 100};
		cout << c.slice({{{4, 6}, {4, 6}}}) << "\n\n";
		cout << c.slice({{{4, 6}, {4, 6}}}) + e << "\n\n";
		c.slice({{{4, 6}, {4, 6}}}) += e;
		cout << c << "\n\n";

		cout << c.slice({{{.step = 3}, {.step = 3}}}) << "\n\n";
		Tensor<int, 2> f({4, 4});
		f.fill(999);
		c.slice({{{.step = 3}, {.step = 3}}}) -= f;
		cout << c << "\n\n";
		c *= -1;
		cout << c << "\n\n";
		auto g{c.slice({{{.step = 4}, {.step = 4}}})};
		cout << g << "\n\n";
		cout << g.transpose({{1, 0}}) << "\n\n";
	}
	return 0;
}

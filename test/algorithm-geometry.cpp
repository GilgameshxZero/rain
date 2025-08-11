#include <rain.hpp>

int main() {
	using namespace Rain::Algorithm::Geometry;

	PointLl a{1, 2}, b{3, 4};
	assert(a.cross(b) == -2);

	LineSegmentLl l1{{0, 0}, {100, 100}}, l2{{100, 0}, {0, 100}},
		l3{{-1, -1}, {100, -1}}, l4{{-1, -1}, {1, 1}};
	std::unordered_set<LineSegmentLl> s1;
	std::set<LineSegmentLl> s2;
	s1.insert(l1);
	s1.insert(l2);
	s2.insert(l1);
	s2.insert(l2);

	assert(l1.intersects(l2));
	assert(l2.intersects(l1));
	assert(!l1.intersects(l3));
	assert(!l3.intersects(l1));
	assert(!l2.intersects(l3));
	assert(!l3.intersects(l2));

	// These lines technically intersect, but our code guarantees that parallel lines never
	// intersect.
	assert(!l1.intersects(l4));
	assert(!l4.intersects(l1));

	return 0;
}

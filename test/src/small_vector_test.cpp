#include "catch.hpp"
#include "helper.h"
#include <perfvect/small_vector.h>
#include <array>
#include <vector>
#include <iostream>
#include <chrono>

using namespace std::literals;
using namespace perfvect;

TEST_CASE("small_vector(), small_vector::size(), small_vector::capacity(), small_vector::empty()") {
	SECTION("construct static variant") {
		small_vector<int, 8> vec;
		CHECK(vec.is_static());
		CHECK(vec.size() == 0u);
		CHECK(vec.capacity() == 8u);
		CHECK(vec.empty());
	}

	SECTION("construct dynamic variant") {
		small_vector<int, 2, 8> vec{1, 2, 3};
		CHECK(vec.is_dynamic());
		CHECK(vec.size() == 3u);
		CHECK(vec.capacity() == 8u);
		CHECK(!vec.empty());
	}
}

TEST_CASE("small_vector(const small_vector&)") {
	SECTION("static variant") {
		small_vector<int, 4> vec({1, 2, 3});
		small_vector<int, 4> copy(vec);
		REQUIRE(copy.size() == 3);
	}

	SECTION("dynamic variant") {
		small_vector<int, 2, 8> vec({1, 2, 3});
		small_vector<int, 2, 8> copy(vec);
		CHECK(!copy.is_static());
		CHECK(copy.capacity() == 8);
		REQUIRE(copy.size() == 3);
	}
}

TEST_CASE("small_vector(small_vector&&)") {
	SECTION("static variant") {
		small_vector<int, 4> vec({1, 2, 3});
		small_vector<int, 4> copy(std::move(vec));
		CHECK(copy.is_static());
		CHECK(vec.size() == 0);
		CHECK(copy.size() == 3);
	}

	SECTION("dynamic variant") {
		small_vector<int, 2, 4> vec({1, 2, 3});
		small_vector<int, 2, 4> copy(std::move(vec));
		CHECK(copy.is_dynamic());
		CHECK(copy.size() == 3);
		CHECK(copy.capacity() == 4);
		CHECK(vec.is_static());
		CHECK(vec.size() == 0);
		CHECK(vec.capacity() == 2);
	}
}

TEST_CASE("small_vector(const small_vector<T, OtherStaticCapacity, OtherDynamicCapacity>&)") {
	SECTION("construct dynamic variant from higher static capacity vector") {
		small_vector<int, 3, 4> vec({1, 2, 3});
		small_vector<int, 2, 4> copy(vec);
		CHECK(copy.is_dynamic());
		CHECK(copy.size() == 3);
		CHECK(copy.capacity() == 4);
		CHECK(copy[0] == 1);
		CHECK(copy[1] == 2);
		CHECK(copy[2] == 3);
	}

	SECTION("construct static variant from lower static capacity vector") {
		small_vector<int, 2, 4> vec({1, 2, 3});
		small_vector<int, 3, 4> copy(vec);
		CHECK(copy.is_static());
		CHECK(copy.size() == 3);
		CHECK(copy.capacity() == 3);
		CHECK(copy[0] == 1);
		CHECK(copy[1] == 2);
		CHECK(copy[2] == 3);
	}
}

TEST_CASE("small_vector(size_type, const T&)") {
	SECTION("static variant") {
		small_vector<int, 3> vec(3, 99);
		CHECK(vec.is_static());
		CHECK(vec.capacity() == 3);
		CHECK(vec.size() == 3);
	}

	SECTION("dynamic variant") {
		small_vector<int, 3, 5> vec(4, 99);
		CHECK(!vec.is_static());
		CHECK(vec.capacity() == 5);
		CHECK(vec.size() == 4);
	}
}

TEST_CASE("small_vector(iterator, iterator)") {
	static auto arr = std::array<int, 3>{1, 2, 3};

	SECTION("static variant") {
		small_vector<int, 4> vec(arr.begin(), arr.end());
		CHECK(vec.is_static());
		CHECK(vec.size() == arr.size());
		CHECK(vec.capacity() == 4);
	}

	SECTION("dynamic variant") {
		small_vector<int, 2, 4> vec(arr.begin(), arr.end());
		CHECK(vec.is_dynamic());
		CHECK(vec.size() == arr.size());
		CHECK(vec.capacity() == 4);
	}

	SECTION("std::initializer_list") {
		small_vector<int, 2, 4> vec{1, 2, 3};
		CHECK(vec.is_dynamic());
		CHECK(vec.size() == 3);
		CHECK(vec.capacity() == 4);
	}
}

TEST_CASE("small_vector::operator=(small_vector&&)") {
	SECTION("move static vectors") {
		small_vector<TestStruct, 2, 4> vec({1, 2});
		TestStruct::setup();
		small_vector<TestStruct, 2, 4> copy(std::move(vec));
		CHECK(TestStruct::moveConstructed == 2);
		CHECK(copy.size() == 2);
		CHECK(copy.is_static());
		CHECK(vec.size() == 0);
		CHECK(vec.is_static());
	}

	SECTION("move dynamic vector") {
		small_vector<TestStruct, 1, 3> vec({1, 2});
		TestStruct::setup();
		small_vector<TestStruct, 1, 3> copy(std::move(vec));

		// std::vector buffers can be swapped so no per-element constructions/assignments occur
		CHECK(TestStruct::constructed == 0);
		CHECK(TestStruct::assigned == 0);
		CHECK(copy.size() == 2);
		CHECK(copy.is_dynamic());
		CHECK(vec.size() == 0);
		CHECK(vec.is_static());
	}
}

TEST_CASE("small_vector::operator=(const small_vector&)") {
	static_vector<std::string, 2> vec;
	auto buff1 = vec.emplace_back("test1"s).data();
	auto buff2 = vec.emplace_back("test2"s).data();
	static_vector<std::string, 2> other;
	other = vec;
	REQUIRE(other.size() == 2);
	CHECK(other[0] == "test1"s);
	CHECK(other[1] == "test2"s);
	CHECK(other[0].data() != buff1);
	CHECK(other[1].data() != buff2);
}

TEST_CASE("small_vector::operator=(std::initializer_list)") {
	small_vector<int, 2, 4> vec;
	vec = {1, 2, 3};
	REQUIRE(vec.size() == 3);
	CHECK(vec[0] == 1);
	CHECK(vec[1] == 2);
	CHECK(vec[2] == 3);
	CHECK(vec.is_dynamic());
	CHECK(vec.capacity() == 4);
}

TEST_CASE("small_vector::swap(small_vector&&)") {
	small_vector<int, 2, 4> vec1{1, 2, 3};
	small_vector<int, 2, 4> vec2{1, 2};
	vec1.swap(vec2);
	CHECK(vec1.size() == 2);
	CHECK(vec1.capacity() == 2);
	CHECK(vec1.is_static());
	CHECK(vec2.size() == 3);
	CHECK(vec2.capacity() == 4);
	CHECK(vec2.is_dynamic());
}
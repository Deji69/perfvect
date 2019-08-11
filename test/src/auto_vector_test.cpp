#include "catch.hpp"
#include "helper.h"
#include <perfvect/small_vector.h>
#include <array>
#include <vector>
#include <iostream>
#include <chrono>

using namespace std::literals;
using namespace perfvect;

TEST_CASE("auto_vector(), auto_vector::size(), auto_vector::capacity(), auto_vector::empty()") {
	SECTION("construct static variant") {
		small_vector<int, 8> vec;
		CHECK(vec.is_static());
		CHECK(vec.size() == 0);
		CHECK(vec.capacity() == 8u);
		vec.push_back(1);
		CHECK(!vec.empty());
	}
}

TEST_CASE("auto_vector(const auto_vector&)") {
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

TEST_CASE("auto_vector(auto_vector&&)") {
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
		CHECK(!copy.is_static());
		CHECK(copy.size() == 3);
		CHECK(copy.capacity() == 4);
		CHECK(!vec.is_static());
		CHECK(vec.size() == 0);
		CHECK(vec.capacity() == 0);
	}
}

TEST_CASE("auto_vector(size_type, const T&)") {
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

TEST_CASE("auto_vector(iterator, iterator)") {
	static auto arr = std::array<int, 3>{1, 2, 3};

	SECTION("static variant") {
		small_vector<int, 4> vec(arr.begin(), arr.end());
		CHECK(vec.is_static());
		CHECK(vec.size() == arr.size());
		CHECK(vec.capacity() == 4);
	}

	SECTION("dynamic variant") {
		small_vector<int, 2, 4> vec(arr.begin(), arr.end());
		CHECK(!vec.is_static());
		CHECK(vec.size() == arr.size());
		CHECK(vec.capacity() == 4);
	}

	SECTION("initializer list") {
		small_vector<int, 2, 4> vec{1, 2, 3};
		CHECK(!vec.is_static());
		CHECK(vec.size() == 3);
		CHECK(vec.capacity() == 4);
	}
}

TEST_CASE("auto_vector::operator=(auto_vector&&)") {
	SECTION("swap static vectors") {
		small_vector<TestStruct, 2, 4> vec({1, 2});
		TestStruct::setup();
		small_vector<TestStruct, 2, 4> copy(std::move(vec));
		CHECK(TestStruct::moveConstructed == 2);
		CHECK(copy.size() == 2);
		CHECK(copy.is_static());
		CHECK(vec.size() == 0);
		CHECK(vec.is_static());
	}

	SECTION("swap dynamic vector") {
		small_vector<TestStruct, 1, 3> vec({1, 2});
		TestStruct::setup();
		small_vector<TestStruct, 1, 3> copy(std::move(vec));
		CHECK(TestStruct::moveConstructed == 2);
		CHECK(copy.size() == 2);
		CHECK(!copy.is_static());
		CHECK(vec.size() == 0);
		CHECK(vec.is_static());
	}
}

TEST_CASE("static_vector::operator=(const static_vector&)") {
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
#include <perfvect/vector.h>
#include <array>
#include "catch.hpp"
#include "helper.h"

using namespace perfvect;

TEST_CASE("vector()") {
	SECTION("default constructor") {
		vector<int> vec;
		CHECK(vec.size() == 0);
		CHECK(vec.capacity() == 0u);
		CHECK(vec.max_size() > 0u);
		CHECK(vec.begin() == vec.end());
		CHECK(vec.cbegin() == vec.cend());
		CHECK(vec.rbegin() == vec.rend());
		CHECK(vec.crbegin() == vec.crend());
	}
}

TEST_CASE("vector(vector&&)") {
	vector<TestStruct> vec({1, 2, 3});
	TestStruct::setup();
	vector<TestStruct> copy(std::move(vec));
	CHECK(vec.size() == 0);
	CHECK(copy.size() == 3);
	CHECK(TestStruct::moveConstructed == 3);
}

TEST_CASE("vector(const vector&)") {
	vector<TestStruct> vec({1, 2, 3});
	TestStruct::setup();
	vector<TestStruct> copy(vec);
	CHECK(vec.size() == 3);
	CHECK(copy.size() == 3);
	CHECK(TestStruct::copyConstructed == 3);
}

TEST_CASE("vector(iterator, iterator)") {
	std::array arr{1, 2, 3};
	vector<int> vec(arr.begin(), arr.end());
	REQUIRE(vec.size() == 3);
	CHECK(vec[0] == 1);
	CHECK(vec[1] == 2);
	CHECK(vec[2] == 3);
}

TEST_CASE("vector(size_type, const value_type&)") {
	vector<int> vec(3, 99);
	REQUIRE(vec.size() == 3);
	CHECK(vec[0] == 99);
	CHECK(vec[1] == 99);
	CHECK(vec[2] == 99);
}

TEST_CASE("vector(std::initializer_list)") {
	vector<int> vec({1, 2, 3});
	REQUIRE(vec.size() == 3);
	CHECK(vec[0] == 1);
	CHECK(vec[1] == 2);
	CHECK(vec[2] == 3);
}

TEST_CASE("~vector()") {
	{
		vector<TestStruct> vec(3, 99);
		TestStruct::setup();
	}
	CHECK(TestStruct::destructed == 3);
}

TEST_CASE("vector::assign(size_type, const value_type&)") {
	SECTION("basic fill assign") {
		vector<TestStruct> vec;
		vec.assign(3, 99);
		CHECK(TestStruct::valueConstructed == 1);
		CHECK(TestStruct::copyConstructed == 3);
		REQUIRE(vec.size() == 3);
		CHECK(vec[0] == 99);
		CHECK(vec[1] == 99);
		CHECK(vec[2] == 99);
	}
	
	SECTION("overwrite fill assign") {
		vector<TestStruct> vec(3, 0);
		TestStruct::setup();
		vec.assign(3, 99);
		CHECK(TestStruct::valueConstructed == 1);
		CHECK(TestStruct::copyAssigned == 3);
		REQUIRE(vec.size() == 3);
		CHECK(vec[0] == 99);
		CHECK(vec[1] == 99);
		CHECK(vec[2] == 99);
	}
}

TEST_CASE("vector::reserve(size_type)") {
	auto vec = vector<TestStruct>();

	SECTION("basic reserve") {
		vec.reserve(4);
		CHECK(vec.capacity() == 4);
	}

	SECTION("re-reserve") {
		vec.reserve(4);
		CHECK(vec.capacity() == 4);
		vec.reserve(8);
		CHECK(vec.capacity() == 8);
	}

	SECTION("reserve moves elements to new allocation") {
		vec.reserve(2);
		vec.emplace_back(1);
		vec.emplace_back(2);
		vec.reserve(4);
		CHECK(vec.capacity() == 4);
		CHECK(vec.size() == 2);
		CHECK(vec[0].wasMoveConstructed);
		CHECK(vec[1].wasMoveConstructed);
	}
}
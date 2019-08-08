#include "catch.hpp"
#include "helper.h"
#include <perfvect/static_vector.h>
#include <array>
#include <vector>
#include <iostream>
#include <chrono>

using namespace std::literals;
using namespace perfvect;

TEST_CASE("small_vector(), small_vector::size(), small_vector::max_size(), small_vector::capacity(), small_vector::empty()")
{
	SECTION("default constructor")
	{
		static_vector<int, 8> vec;
		CHECK(vec.size() == 0);
		CHECK(vec.capacity() == 8u);
		CHECK(vec.max_size() == 8u);
		CHECK(vec.begin() == vec.end());
		CHECK(vec.cbegin() == vec.cend());
		CHECK(vec.rbegin() == vec.rend());
		CHECK(vec.crbegin() == vec.crend());
	}

	SECTION("copy constructor")
	{
		static_vector<int, 8> vec;
		vec.push_back(1);
		vec.push_back(2);
		vec.push_back(3);
		static_vector<int, 8> copy(vec);
		REQUIRE(copy.size() == 3);
		CHECK(copy[0] == 1);
		CHECK(copy[1] == 2);
		CHECK(copy[2] == 3);
	}

	SECTION("move constructor")
	{
		static_vector<int, 8> vec;
		vec.push_back(1);
		vec.push_back(2);
		vec.push_back(3);
		static_vector<int, 8> copy(std::move(vec));
		REQUIRE(copy.size() == 3);
		CHECK(copy[0] == 1);
		CHECK(copy[1] == 2);
		CHECK(copy[2] == 3);
	}

	SECTION("move construction moves contained buffers")
	{
		static_vector<std::string, 2> vec;
		vec.push_back("test1"s);
		vec.push_back("test2"s);
		auto buff1 = vec[0].data();
		auto buff2 = vec[1].data();
		static_vector<std::string, 2> other(std::move(vec));
		CHECK(other[0].data() == buff1);
		CHECK(other[1].data() == buff2);
	}

	SECTION("sanity check capacity methods")
	{
		static_vector<int, 3> vec;
		REQUIRE(vec.capacity() == 3);
		REQUIRE(vec.max_size() == 3);
		REQUIRE(vec.empty());
		CHECK(vec.size() == 0);
		vec.push_back(1);
		CHECK(!vec.empty());
		CHECK(vec.size() == 1);
		vec.push_back(1);
		CHECK(vec.size() == 2);
		vec.push_back(1);
		CHECK(vec.size() == 3);
	}
}

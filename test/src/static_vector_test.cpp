#include "catch.hpp"
#include "helper.h"
#include <perfvect/static_vector.h>
#include <array>
#include <vector>
#include <iostream>
#include <chrono>

using namespace std::literals;
using namespace perfvect;

TEST_CASE("static_vector(), static_vector::size(), static_vector::max_size(), static_vector::capacity(), static_vector::empty()") {
	SECTION("default constructor") {
		static_vector<int, 8> vec;
		CHECK(vec.size() == 0);
		CHECK(vec.capacity() == 8u);
		CHECK(vec.max_size() == 8u);
		CHECK(vec.begin() == vec.end());
		CHECK(vec.cbegin() == vec.cend());
		CHECK(vec.rbegin() == vec.rend());
		CHECK(vec.crbegin() == vec.crend());
	}

	SECTION("copy constructor") {
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

	SECTION("move constructor") {
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

	SECTION("move construction swaps contained buffers") {
		static_vector<TestStruct, 2> vec(2, TestStruct{99});
		TestStruct::setup();
		static_vector<TestStruct, 2> other(std::move(vec));
		CHECK(other.size() == 2);
		CHECK(TestStruct::constructed == 2);
		CHECK(TestStruct::moveConstructed == 2);
		CHECK(other[0].wasMoveConstructed);
		CHECK(other[1].wasMoveConstructed);
		CHECK(vec.empty());
	}

	SECTION("sanity check capacity methods") {
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

TEST_CASE("static_vector::static_vector(size_type, const value_type&)") {
	static_vector<int, 5> vec(3, 99);
	CHECK(vec.capacity() == 5);
	REQUIRE(vec.size() == 3);
	CHECK(vec[0] == 99);
	CHECK(vec[1] == 99);
	CHECK(vec[2] == 99);
}

TEST_CASE("static_vector::static_vector(iterator, iterator)") {
	SECTION("trivial range construction") {
		static const std::array<int, 4> data{1, 2, 3, 4};
		static_vector<int, 4> vec(data.begin(), data.end());
		REQUIRE(vec.size() == 4);
		CHECK(vec[0] == 1);
		CHECK(vec[1] == 2);
		CHECK(vec[2] == 3);
		CHECK(vec[3] == 4);
	}

	SECTION("non-trivial range construction") {
		static const std::array<TestStruct, 3> data{{{1}, {2}, {3}}};
		static_vector<TestStruct, 3> vec(data.begin(), data.end());
		REQUIRE(vec.size() == 3);
		CHECK(vec[0].value == 1);
		CHECK(vec[1].value == 2);
		CHECK(vec[2].value == 3);
	}
}

TEST_CASE("static_vector::static_vector(std::initialier_list)") {
	static_vector<int, 4> vec({1, 2, 3, 4});
	REQUIRE(vec.size() == 4);
	CHECK(vec[0] == 1);
	CHECK(vec[1] == 2);
	CHECK(vec[2] == 3);
	CHECK(vec[3] == 4);
}

TEST_CASE("static_vector::~static_vector()") {
	{
		static_vector<TestStruct, 4> vec({1, 2, 3, 4});
		TestStruct::setup();
	}
	CHECK(TestStruct::destructed == 4);
}

TEST_CASE("static_vector::operator=(static_vector&&)") {
	SECTION("swapped when lengths are equal") {
		static_vector<TestStruct, 2> vec{1, 2};
		static_vector<TestStruct, 2> other{3, 4};
		TestStruct::setup();
		other = std::move(vec);
		REQUIRE(other.size() == 2);
		CHECK(other[0].value == 1);
		CHECK(other[1].value == 2);
		CHECK(vec[0].value == 3);
		CHECK(vec[1].value == 4);
	}

	SECTION("move constructs into empty") {
		static_vector<TestStruct, 2> vec{1, 2};
		static_vector<TestStruct, 2> other;
		TestStruct::setup();
		other = std::move(vec);
		REQUIRE(other.size() == 2);
		CHECK(other[0].wasMoveConstructed);
		CHECK(other[1].wasMoveConstructed);
	}

	SECTION("destructs uninitialised elements") {
		static_vector<TestStruct, 2> other{1, 2};

		{
			static_vector<TestStruct, 2> vec;
			TestStruct::setup();
			other = std::move(vec);
			CHECK(other.size() == 0);
			CHECK(TestStruct::destructed == 2);
			TestStruct::setup();
		}

		CHECK(TestStruct::destructed == 2);
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

TEST_CASE("static_vector::operator=(std::initializer_list)") {
	static_vector<std::string, 2> vec;
	vec = {"test1"s, "test2"s};
	REQUIRE(vec.size() == 2);
	CHECK(vec[0] == "test1"s);
	CHECK(vec[1] == "test2"s);
}

TEST_CASE("static_vector::operator[](size_type)") {
	static_vector<int, 3> vec({1, 2, 3});
	CHECK(vec[0] == 1);
	CHECK(vec[1] == 2);
	CHECK(vec[2] == 3);
}

TEST_CASE("static_vector::at(size_type)") {
	auto vec = static_vector<int, 3>({1, 2, 3});

	SECTION("can access element") {
		CHECK(vec.at(0) == 1);
		CHECK(vec.at(1) == 2);
		CHECK(vec.at(2) == 3);
	}

	SECTION("invalid access throws std::out_of_range exception") {
		REQUIRE_THROWS_AS(vec.at(3), std::out_of_range);
	}
}

TEST_CASE("static_vector::assign(size_type, const value_type&)") {
	SECTION("fill assignment") {
		static_vector<int, 3> vec;
		vec.assign(2, 99);
		REQUIRE(vec.size() == 2);
		CHECK(vec[0] == 99);
		CHECK(vec[1] == 99);
	}
	
	SECTION("old elements are overwritten or destructed") {
		static_vector<TestStruct, 3> vec{1, 2, 3};
		bool destructed = false;
		vec[2].onDestruct = [&destructed](){ destructed = true; };
		TestStruct::setup();
		vec.assign(2, 99);
		CHECK(destructed);
		CHECK(TestStruct::copyAssigned == 3);
	}

	SECTION("new elements are initialised") {
		static_vector<TestStruct, 2> vec{};
		TestStruct::setup();
		vec.assign(2, 99);
		CHECK(TestStruct::copyConstructed == 2);
	}
}

TEST_CASE("static_vector::assign(iterator, iterator)") {
	SECTION("trivial range assignment") {
		static const std::array<int, 3> data{1, 2, 3};
		static_vector<int, 3> vec;
		vec.assign(data.begin(), data.end());
		REQUIRE(vec.size() == 3);
		CHECK(vec[0] == 1);
		CHECK(vec[1] == 2);
		CHECK(vec[2] == 3);
	}

	SECTION("non-trivial range assignment") {
		static std::array<TestStruct, 6> data;
		static_vector<TestStruct, 6> vec(data.begin(), data.end() - 2);
		TestStruct::setup();

		SECTION("underflowing copy assignment destructing elements") {
			vec.assign(data.begin(), data.begin() + 2);
			CHECK(TestStruct::destructed == 2);
			CHECK(TestStruct::copyAssigned == 2);
			CHECK(TestStruct::copyConstructed == 0);
			CHECK(TestStruct::moveConstructed == 0);
			CHECK(TestStruct::moveAssigned == 0);
		}

		SECTION("overflowing copy assignment constructing elements") {
			vec.assign(data.begin(), data.end());
			CHECK(TestStruct::destructed == 0);
			CHECK(TestStruct::copyAssigned == 4);
			CHECK(TestStruct::copyConstructed == 2);
			CHECK(TestStruct::moveConstructed == 0);
			CHECK(TestStruct::moveAssigned == 0);
		}

		SECTION("move assignment and construction") {
			vec.assign(std::make_move_iterator(data.begin()), std::make_move_iterator(data.end()));
			CHECK(TestStruct::destructed == 0);
			CHECK(TestStruct::copyAssigned == 0);
			CHECK(TestStruct::copyConstructed == 0);
			CHECK(TestStruct::moveConstructed == 2);
			CHECK(TestStruct::moveAssigned == 4);
		}
	}
}

TEST_CASE("static_vector::assign(std::initializer_list)") {
	static_vector<int, 3> vec;
	vec.assign({1, 2, 3});
	REQUIRE(vec.size() == 3);
	CHECK(vec[0] == 1);
	CHECK(vec[1] == 2);
	CHECK(vec[2] == 3);
}

TEST_CASE("static_vector::fill(const value_type&)") {
	static_vector<int, 3> vec;
	vec.fill(99);
	REQUIRE(vec.size() == 3);
	CHECK(vec[0] == 99);
	CHECK(vec[1] == 99);
	CHECK(vec[2] == 99);
}

TEST_CASE("static_vector::swap(static_vector&)") {
	static_vector<int, 3> vec({1, 2, 3});
	static_vector<int, 3> other({11, 12, 13});
	vec.swap(other);
	REQUIRE(vec.size() == 3);
	REQUIRE(other.size() == 3);
	CHECK(vec[0] == 11);
	CHECK(vec[1] == 12);
	CHECK(vec[2] == 13);
	CHECK(other[0] == 1);
	CHECK(other[1] == 2);
	CHECK(other[2] == 3);
}

TEST_CASE("static_vector::front(), static_vector::back()") {
	static_vector<int, 8> vec;
	vec.push_back(1);
	vec.push_back(2);
	vec.push_back(3);

	SECTION("front") {
		REQUIRE(vec.front() == 1);
	}

	SECTION("back") {
		REQUIRE(vec.back() == 3);
	}
}

TEST_CASE("static_vector::insert(iterator, const value_type&)") {
	auto vec = static_vector<int, 5>();

	SECTION("insert into front of vector") {
		vec.push_back(2);
		vec.push_back(3);
		vec.push_back(4);
		vec.insert(vec.begin(), 1);
		REQUIRE(vec.size() == 4);
		CHECK(vec[0] == 1);
		CHECK(vec[1] == 2);
		CHECK(vec[2] == 3);
		CHECK(vec[3] == 4);
	}

	SECTION("insert into middle of vector") {
		vec.push_back(1);
		vec.push_back(3);
		vec.insert(vec.begin() + 1, 2);
		REQUIRE(vec.size() == 3);
		CHECK(vec[0] == 1);
		CHECK(vec[1] == 2);
		CHECK(vec[2] == 3);
	}

	SECTION("insert into end of vector") {
		vec.push_back(1);
		vec.push_back(2);
		vec.insert(vec.end(), 3);
		REQUIRE(vec.size() == 3);
		CHECK(vec[0] == 1);
		CHECK(vec[1] == 2);
		CHECK(vec[2] == 3);
	}

	SECTION("insert into empty vector") {
		vec.insert(vec.begin(), 99);
		REQUIRE(vec.size() == 1);
		CHECK(vec[0] == 99);
	}
}

TEST_CASE("static_vector::insert(iterator, value_type&&)") {
	static_vector<TestStruct, 3> vec{1, 3};
	TestStruct test(2);
	TestStruct::setup();
	auto it = vec.insert(vec.begin() + 1, std::move(test));
	REQUIRE(vec.size() == 3);
	CHECK(vec.begin() + 1 == it);
	CHECK(vec[1].value == 2);
	CHECK(test.wasMoveConstructedFrom);
}

TEST_CASE("static_vector::insert(iterator, size_type, const value_type&)") {
	auto vec = static_vector<int, 5>();

	SECTION("insert into middle of vector") {
		vec.push_back(1);
		vec.push_back(5);
		auto it = vec.insert(vec.begin() + 1, 3, 99);
		REQUIRE(vec.size() == 5);
		CHECK(vec[0] == 1);
		CHECK(vec[1] == 99);
		CHECK(vec[2] == 99);
		CHECK(vec[3] == 99);
		CHECK(vec[4] == 5);
		REQUIRE(*it == 99);
		CHECK(it - vec.begin() == 1);
	}

	SECTION("insert into empty vector") {
		auto it = vec.insert(vec.begin(), 3, 1);
		REQUIRE(vec.size() == 3);
		CHECK(vec[0] == 1);
		CHECK(vec[1] == 1);
		CHECK(vec[2] == 1);
		REQUIRE(*it == 1);
		CHECK(it == vec.begin());
	}

	SECTION("insert into end of vector") {
		vec.push_back(1);
		auto it = vec.insert(vec.end(), 3, 99);
		CHECK(vec[0] == 1);
		CHECK(vec[1] == 99);
		CHECK(vec[2] == 99);
		CHECK(vec[3] == 99);
		CHECK(*it == 99);
		CHECK(vec.size() == 4);
	}
}

TEST_CASE("static_vector::insert(iterator, iterator, iterator)") {
	auto vec = static_vector<int, 5>();

	SECTION("insert into front of vector") {
		vec.push_back(3);
		vec.push_back(4);
		auto it = vec.insert(vec.begin(), {1, 2});
		CHECK(vec[0] == 1);
		CHECK(vec[1] == 2);
		CHECK(vec[2] == 3);
		CHECK(vec[3] == 4);
		CHECK(*it == 1);
	}

	SECTION("insert into middle of vector") {
		vec.push_back(1);
		vec.push_back(5);
		auto other = std::vector<int>({2, 3, 4});
		auto it = vec.insert(vec.begin() + 1, other.begin(), other.end());
		CHECK(vec[0] == 1);
		CHECK(vec[1] == 2);
		CHECK(vec[2] == 3);
		CHECK(vec[3] == 4);
		CHECK(vec[4] == 5);
		CHECK(*it == 2);
	}

	SECTION("insert into empty vector") {
		auto other = std::vector<int>({1, 2, 3});
		auto it = vec.insert(vec.begin(), other.begin(), other.end());
		CHECK(vec[0] == 1);
		CHECK(vec[1] == 2);
		CHECK(vec[2] == 3);
		CHECK(*it == 1);
	}
}

TEST_CASE("static_vector::insert(iterator, std::initializer_list") {
	static_vector<int, 5> vec;
	vec.push_back(1);
	vec.push_back(5);
	vec.insert(vec.begin() + 1, {2, 3, 4});
	REQUIRE(vec.size() == 5);
	CHECK(vec[0] == 1);
	CHECK(vec[1] == 2);
	CHECK(vec[2] == 3);
	CHECK(vec[3] == 4);
	CHECK(vec[4] == 5);
}

TEST_CASE("static_vector::emplace(iterator, ...)") {
	struct TestStruct {
		int value = 0;
	};

	static_vector<TestStruct, 4> vec;

	SECTION("can emplace into empty vector") {
		auto it = vec.emplace(vec.end(), 1);
		REQUIRE(vec.size() == 1);
		CHECK(vec[0].value == 1);
		CHECK(it == vec.begin());
	}

	SECTION("can emplace into back") {
		vec.emplace(vec.end(), 1);
		auto it = vec.emplace(vec.end(), 2);
		REQUIRE(vec.size() == 2);
		CHECK(vec[0].value == 1);
		CHECK(vec[1].value == 2);
		CHECK(it == vec.end() - 1);
	}

	SECTION("can emplace into front") {
		vec.emplace(vec.end(), 2);
		vec.emplace(vec.end(), 3);
		vec.emplace(vec.end(), 4);
		auto it = vec.emplace(vec.begin(), 1);
		REQUIRE(vec.size() == 4);
		CHECK(vec[0].value == 1);
		CHECK(it == vec.begin());
	}

	SECTION("can emplace into middle") {
		vec.emplace(vec.end(), 1);
		vec.emplace(vec.end(), 2);
		vec.emplace(vec.end(), 4);
		auto it = vec.emplace(vec.begin() + 2, 3);
		REQUIRE(vec.size() == 4);
		CHECK(vec[2].value == 3);
		CHECK(it == vec.begin() + 2);
	}
}

TEST_CASE("static_vector::emplace_back(...)") {
	struct TestStruct {
		int value = 0;
	};

	static_vector<TestStruct, 4> vec;
	vec.emplace_back(1);
	REQUIRE(vec.size() == 1);
	CHECK(vec[0].value == 1);
	vec.emplace_back(2);
	REQUIRE(vec.size() == 2);
	CHECK(vec[1].value == 2);
}

TEST_CASE("static_vector::push_back(value_type)") {
	static_vector<int, 8> vec;

	SECTION("check size growth") {
		for (auto i = 0u; i < 8u; ++i) {
			REQUIRE(vec.size() == i);
			vec.push_back(1);
		}

		REQUIRE(vec.size() == 8);
	}

	SECTION("check value access") {
		vec.push_back(1);
		vec.push_back(2);
		vec.push_back(3);
		REQUIRE(vec[0] == 1);
		REQUIRE(vec[1] == 2);
		REQUIRE(vec[2] == 3);
	}
}

TEST_CASE("static_vector::push_back(value_type&&)") {
	static_vector<TestStruct, 4> vec;
	TestStruct test;
	TestStruct::setup();
	vec.push_back(std::move(test));
	CHECK(test.wasMoveConstructedFrom);
	CHECK(vec.back().wasMoveConstructed);
	CHECK(TestStruct::constructed == 1);
	CHECK(TestStruct::moveConstructed == 1);
}

TEST_CASE("static_vector::pop_back()") {
	SECTION("reduces size") {
		static_vector<int, 8> vec;

		for (auto i = 0u; i < 8u; ++i) {
			vec.push_back(1);
		}

		for (auto i = 0u; i < 8u; ++i) {
			REQUIRE(vec.size() == (8 - i));
			vec.pop_back();
		}

		REQUIRE(vec.size() == 0);
	}

	SECTION("destroys elements") {
		static_vector<TestStruct, 3> vec(3, TestStruct{});
		TestStruct::setup();
		vec.pop_back();
		REQUIRE(TestStruct::destructed == 1);
		vec.pop_back();
		REQUIRE(TestStruct::destructed == 2);
		vec.pop_back();
		REQUIRE(TestStruct::destructed == 3);
	}
}

TEST_CASE("static_vector::erase(iterator)") {
	static_vector<int, 3> vec{1, 2, 3};

	SECTION("can erase from front") {
		vec.erase(vec.begin());
		REQUIRE(vec.size() == 2);
		CHECK(vec[0] == 2);
		CHECK(vec[1] == 3);
	}

	SECTION("can erase from middle") {
		vec.erase(vec.begin() + 1);
		REQUIRE(vec.size() == 2);
		CHECK(vec[0] == 1);
		CHECK(vec[1] == 3);
	}

	SECTION("can erase from back") {
		vec.erase(vec.begin() + 2);
		REQUIRE(vec.size() == 2);
		CHECK(vec[0] == 1);
		CHECK(vec[1] == 2);
	}
}

TEST_CASE("static_vector::erase(iterator, iterator)") {
	static_vector<int, 5> vec{1, 2, 3, 4, 5};

	SECTION("can remove 3 from front") {
		vec.erase(vec.begin(), vec.begin() + 3);
		REQUIRE(vec.size() == 2);
		CHECK(vec[0] == 4);
		CHECK(vec[1] == 5);
	}

	SECTION("can remove 3 from middle") {
		vec.erase(vec.begin() + 1, vec.begin() + 4);
		REQUIRE(vec.size() == 2);
		CHECK(vec[0] == 1);
		CHECK(vec[1] == 5);
	}

	SECTION("can remove 3 from back") {
		vec.erase((vec.rbegin() + 3).base(), vec.rbegin().base());
		REQUIRE(vec.size() == 2);
		CHECK(vec[0] == 1);
		CHECK(vec[1] == 2);
	}
}

TEST_CASE("static_vector::clear()") {
	static auto destructed = 0;
	struct TestStruct {
		~TestStruct() {
			++destructed;
		}
		int value = 0;
	};

	static_vector<TestStruct, 3> vec;
	vec.emplace_back(1);
	vec.emplace_back(1);
	vec.emplace_back(1);
	vec.clear();
	CHECK(vec.size() == 0);
	CHECK(destructed == 3);
}

TEST_CASE("static_vector::data()") {
	static_vector<int, 3> vec({1, 2, 3});
	CHECK(vec.data()[0] == 1);
	CHECK(vec.data()[1] == 2);
	CHECK(vec.data()[2] == 3);
}

TEST_CASE("static_vector::begin(), static_vector::end()") {
	static_vector<int, 8> vec;
	vec.push_back(1);
	vec.push_back(2);
	vec.push_back(3);

	SECTION("begin returns iterator to front") {
		REQUIRE(*vec.begin() == 1);
	}

	SECTION("end returns iterator to one past back") {
		REQUIRE(*std::prev(vec.end()) == 3);
	}

	SECTION("can reach end from begin") {
		auto it = vec.begin();
		++it;
		++it;
		++it;
		REQUIRE(it == vec.end());
	}
}

TEST_CASE("static_vector::rbegin(), static_vector::rend()") {
	static_vector<int, 8> vec;
	vec.push_back(1);
	vec.push_back(2);
	vec.push_back(3);

	SECTION("rbegin returns reverse iterator to back") {
		REQUIRE(*vec.rbegin() == 3);
	}

	SECTION("rend returns reverse iterator to one before front") {
		REQUIRE(*std::prev(vec.rend()) == 1);
	}

	SECTION("can reach rend from rbegin") {
		auto it = vec.rbegin();
		++it;
		++it;
		++it;
		REQUIRE(it == vec.rend());
	}
}

TEST_CASE("static_vector::cbegin(), static_vector::cend()") {
	static_vector<int, 8> vec;
	vec.push_back(1);
	vec.push_back(2);
	vec.push_back(3);

	SECTION("cbegin returns iterator to front") {
		REQUIRE(*vec.cbegin() == 1);
	}

	SECTION("cend returns iterator to one past back") {
		REQUIRE(*std::prev(vec.cend()) == 3);
	}

	SECTION("can reach cend from cbegin") {
		auto it = vec.cbegin();
		++it;
		++it;
		++it;
		REQUIRE(it == vec.cend());
	}
}

TEST_CASE("static_vector::crbegin(), static_vector::crend()") {
	static_vector<int, 8> vec;
	vec.push_back(1);
	vec.push_back(2);
	vec.push_back(3);

	SECTION("crbegin returns iterator to back") {
		REQUIRE(*vec.crbegin() == 3);
	}

	SECTION("crend returns iterator to one before front") {
		REQUIRE(*std::prev(vec.crend()) == 1);
	}

	SECTION("can reach crend from crbegin") {
		auto it = vec.crbegin();
		++it;
		++it;
		++it;
		REQUIRE(it == vec.crend());
	}
}
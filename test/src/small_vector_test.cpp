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

TEST_CASE("small_vector::assign(size_type, const T&)") {
	SECTION("fill assign empty static vector") {
		small_vector<int, 3> vec;
		vec.assign(3, 99);
		CHECK(vec.is_static());
		REQUIRE(vec.size() == 3);
		CHECK(vec[0] == 99);
		CHECK(vec[1] == 99);
		CHECK(vec[2] == 99);
	}

	SECTION("fill assign non-empty static vector") {
		small_vector<int, 3> vec{1, 2, 3};
		vec.assign(3, 99);
		CHECK(vec.is_static());
		REQUIRE(vec.size() == 3);
		CHECK(vec[0] == 99);
		CHECK(vec[1] == 99);
		CHECK(vec[2] == 99);
	}

	SECTION("fill assign empty dynamic vector") {
		small_vector<int, 2> vec;
		vec.assign(3, 99);
		CHECK(vec.is_dynamic());
		REQUIRE(vec.size() == 3);
		CHECK(vec[0] == 99);
		CHECK(vec[1] == 99);
		CHECK(vec[2] == 99);
	}

	SECTION("fill assign non-empty dynamic vector") {
		small_vector<int, 2> vec{1, 2, 3};
		vec.assign(2, 99);
		CHECK(vec.is_dynamic());
		REQUIRE(vec.size() == 2);
		CHECK(vec[0] == 99);
		CHECK(vec[1] == 99);
	}

	SECTION("fill assign non-empty static to dynamic vector") {
		small_vector<int, 2> vec{1, 2};
		vec.assign(3, 99);
		CHECK(vec.is_dynamic());
		REQUIRE(vec.size() == 3);
		CHECK(vec[0] == 99);
		CHECK(vec[1] == 99);
		CHECK(vec[2] == 99);
	}
}

TEST_CASE("small_vector::begin(), small_vector::end()") {
	small_vector<int, 3> stat({1, 2, 3});
	small_vector<int, 3> dyn({1, 2, 3, 4});

	SECTION("begin() returns iterator to front") {
		SECTION("static variant") {
			CHECK(*stat.begin() == 1);
		}

		SECTION("dynamic variant") {
			CHECK(*dyn.begin() == 1);
		}
	}

	SECTION("end() returns iterator to one past back") {
		SECTION("static variant") {
			CHECK(*std::prev(stat.end()) == 3);
		}

		SECTION("dynamic variant") {
			CHECK(*std::prev(dyn.end()) == 4);
		}
	}

	SECTION("can reach end() from begin()") {
		SECTION("static variant") {
			auto it = stat.begin() + 3;
			CHECK(it == stat.end());
		}

		SECTION("dynamic variant") {
			auto it = dyn.begin() + 4;
			CHECK(it == dyn.end());
		}
	}
}

TEST_CASE("small_vector::rbegin(), small_vector::rend()") {
	small_vector<int, 3> stat({1, 2, 3});
	small_vector<int, 3> dyn({1, 2, 3, 4});

	SECTION("rbegin() returns iterator to back") {
		SECTION("static variant") {
			CHECK(*stat.rbegin() == 3);
		}

		SECTION("dynamic variant") {
			CHECK(*dyn.rbegin() == 4);
		}
	}

	SECTION("rend() returns iterator to one before front") {
		SECTION("static variant") {
			CHECK(*std::prev(stat.rend()) == 1);
		}

		SECTION("dynamic variant") {
			CHECK(*std::prev(dyn.rend()) == 1);
		}
	}

	SECTION("can reach rend() from rbegin()") {
		SECTION("static variant") {
			auto it = stat.rbegin() + 3;
			CHECK(it == stat.rend());
		}

		SECTION("dynamic variant") {
			auto it = dyn.rbegin() + 4;
			CHECK(it == dyn.rend());
		}
	}
}

TEST_CASE("small_vector::cbegin(), small_vector::cend()") {
	small_vector<int, 3> stat({1, 2, 3});
	small_vector<int, 3> dyn({1, 2, 3, 4});

	SECTION("cbegin() returns iterator to front") {
		SECTION("static variant") {
			CHECK(*stat.cbegin() == 1);
		}

		SECTION("dynamic variant") {
			CHECK(*dyn.cbegin() == 1);
		}
	}

	SECTION("cend() returns iterator to one past back") {
		SECTION("static variant") {
			CHECK(*std::prev(stat.cend()) == 3);
		}

		SECTION("dynamic variant") {
			CHECK(*std::prev(dyn.cend()) == 4);
		}
	}

	SECTION("can reach cend() from cbegin()") {
		SECTION("static variant") {
			auto it = stat.cbegin() + 3;
			CHECK(it == stat.cend());
		}

		SECTION("dynamic variant") {
			auto it = dyn.cbegin() + 4;
			CHECK(it == dyn.cend());
		}
	}
}

TEST_CASE("small_vector::crbegin(), small_vector::crend()") {
	small_vector<int, 3> stat({1, 2, 3});
	small_vector<int, 3> dyn({1, 2, 3, 4});

	SECTION("crbegin() returns iterator to back") {
		SECTION("static variant") {
			CHECK(*stat.crbegin() == 3);
		}

		SECTION("dynamic variant") {
			CHECK(*dyn.crbegin() == 4);
		}
	}

	SECTION("crend() returns iterator to one before front") {
		SECTION("static variant") {
			CHECK(*std::prev(stat.crend()) == 1);
		}

		SECTION("dynamic variant") {
			CHECK(*std::prev(dyn.crend()) == 1);
		}
	}

	SECTION("can reach crend() from crbegin()") {
		SECTION("static variant") {
			auto it = stat.crbegin() + 3;
			CHECK(it == stat.crend());
		}

		SECTION("dynamic variant") {
			auto it = dyn.crbegin() + 4;
			CHECK(it == dyn.crend());
		}
	}
}

TEST_CASE("small_vector::at(size_type)") {
	auto stat = small_vector<int, 3>({1, 2, 3});
	auto dyn  = small_vector<int, 2>({1, 2, 3});

	SECTION("can access element") {
		SECTION("static variant") {
			CHECK(stat.at(0) == 1);
			CHECK(stat.at(1) == 2);
			CHECK(stat.at(2) == 3);
		}

		SECTION("dynamic variant") {
			CHECK(dyn.at(0) == 1);
			CHECK(dyn.at(1) == 2);
			CHECK(dyn.at(2) == 3);
		}
	}

	SECTION("invalid access throws std::out_of_range exception") {
		SECTION("static variant") {
			REQUIRE_THROWS_AS(stat.at(3), std::out_of_range);
		}

		SECTION("dynamic variant") {
			REQUIRE_THROWS_AS(dyn.at(3), std::out_of_range);
		}
	}
}

TEST_CASE("small_vector::front()") {
	auto stat = small_vector<int, 3>({1, 2, 3});
	auto dyn = small_vector<int, 2>({1, 2, 3});
	CHECK(stat.front() == 1);
	CHECK(dyn.front() == 1);
}

TEST_CASE("small_vector::back()") {
	auto stat = small_vector<int, 3>({1, 2, 3});
	auto dyn = small_vector<int, 2>({1, 2, 3});
	CHECK(stat.back() == 3);
	CHECK(dyn.back() == 3);
}

TEST_CASE("small_vector::reserve(size_type)") {
	auto vec = small_vector<int, 3, 6>({1, 2, 3});
	vec.reserve(4);
	CHECK(vec.is_dynamic());
	CHECK(vec.capacity() == 6);
	vec.reserve(8);
	CHECK(vec.capacity() == 8);
}

TEST_CASE("small_vector::shrink_to_fit()") {
	auto vec = small_vector<int, 2, 4>({1});
	vec.shrink_to_fit();
	CHECK(vec.capacity() == 2);
	
	vec = {1, 2, 3};
	vec.shrink_to_fit();
	REQUIRE(vec.is_dynamic());
	CHECK(vec.capacity() >= 3);

	vec = {1, 2};
	vec.shrink_to_fit();
	REQUIRE(vec.is_static());
	CHECK(vec.capacity() == 2);
}

TEST_CASE("small_vector::insert(const_iterator, const T&)") {
	auto vec = small_vector<int, 2, 4>();
	vec.insert(vec.end(), 3);
	REQUIRE(vec.size() == 1);
	CHECK(vec.capacity() == 2);
	
	vec.insert(vec.begin(), 1);
	REQUIRE(vec.size() == 2);
	CHECK(vec.capacity() == 2);
	
	vec.insert(vec.begin() + 1, 2);
	REQUIRE(vec.size() == 3);
	CHECK(vec.capacity() == 4);
	CHECK(vec[0] == 1);
	CHECK(vec[1] == 2);
	CHECK(vec[2] == 3);
}

TEST_CASE("small_vector::insert(const_iterator, T&&)") {
	auto arr = std::array<TestStruct, 3>{};
	auto vec = small_vector<TestStruct, 2, 4>();
	auto it = vec.insert(vec.end(), std::move(arr[2]));
	CHECK(it->wasMoveConstructed);
	it = vec.insert(vec.begin(), std::move(arr[0]));
	CHECK(it->wasMoveConstructed);
	it = vec.insert(vec.begin() + 1, std::move(arr[1]));
	CHECK(it->wasMoveConstructed);
}
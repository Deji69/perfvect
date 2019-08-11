#ifndef PERFVECT_TEST_HELPER_H
#define PERFVECT_TEST_HELPER_H

// Useful for testing how the containers manage their types i.e. whether they construct, assign, move or copy them.
// The static vars will count the total number of constructions/assignments/moves/copies since the last setup() call.
struct TestStruct {
	static unsigned int constructed;
	static unsigned int assigned;
	static unsigned int destructed;
	static unsigned int moveConstructed;
	static unsigned int moveAssigned;
	static unsigned int copyConstructed;
	static unsigned int copyAssigned;
	
	static auto setup() {
		constructed = 0;
		assigned = 0;
		destructed = 0;
		moveConstructed = 0;
		moveAssigned = 0;
		copyConstructed = 0;
		copyAssigned = 0;
	}

	TestStruct() noexcept { ++constructed; }
	TestStruct(int value)  noexcept: value(value) { ++constructed; }
	TestStruct(TestStruct&& other) noexcept : value(other.value), wasMoveConstructed(true) {
		++constructed;
		++moveConstructed;
		other.wasMoveConstructedFrom = true;
	}
	TestStruct(const TestStruct& other) noexcept : value(other.value), wasCopyConstructed(true) {
		++constructed;
		++copyConstructed;
		other.wasCopyConstructedFrom = true;
	}
	~TestStruct() { ++destructed; }

	auto operator=(TestStruct&& other) noexcept->TestStruct& {
		++assigned;
		++moveAssigned;
		value = other.value;
		wasMoveAssigned = true;
		other.wasMoveAssignedFrom = true;
		return *this;
	}

	auto operator=(const TestStruct& other) noexcept->TestStruct& {
		++assigned;
		++copyAssigned;
		value = other.value;
		wasCopyAssigned = true;
		other.wasCopyAssignedFrom = true;
		return *this;
	}

	int value = 0;
	bool wasMoveConstructed = false;
	bool wasMoveAssigned = false;
	bool wasCopyConstructed = false;
	bool wasCopyAssigned = false;
	bool wasMoveConstructedFrom = false;
	bool wasMoveAssignedFrom = false;
	mutable bool wasCopyConstructedFrom = false;
	mutable bool wasCopyAssignedFrom = false;
};

inline unsigned int TestStruct::constructed = 0;
inline unsigned int TestStruct::assigned = 0;
inline unsigned int TestStruct::destructed = 0;
inline unsigned int TestStruct::moveConstructed = 0;
inline unsigned int TestStruct::moveAssigned = 0;
inline unsigned int TestStruct::copyConstructed = 0;
inline unsigned int TestStruct::copyAssigned = 0;

#endif
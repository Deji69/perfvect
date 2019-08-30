#ifndef PERFVECT_STATIC_VECTOR_H
#define PERFVECT_STATIC_VECTOR_H

#include "iterator.h"
#include "vector.h"
#include <cstddef>
#include <new>
#include <type_traits>
#include <vector>

namespace perfvect {

template<typename T, std::size_t Capacity>
class static_vector : public static_vector_base<T> {
	template<typename T, std::size_t OtherCapacity>
	friend class static_vector;

	using base_t = static_vector_base<T>;

public:
	using value_type = typename base_t::value_type;
	using size_type = typename base_t::size_type;
	using difference_type = typename base_t::difference_type;
	using reference = typename base_t::reference;
	using const_reference = typename base_t::const_reference;
	using pointer = typename base_t::pointer;
	using const_pointer = typename base_t::const_pointer;
	using iterator = typename base_t::iterator;
	using const_iterator = typename base_t::const_iterator;
	using reverse_iterator = typename base_t::reverse_iterator;
	using const_reverse_iterator = typename base_t::const_reverse_iterator;

public:
	constexpr static_vector() noexcept : base_t(reinterpret_cast<pointer>(&m_storage[0]), Capacity) {}
	
	template<typename InputIt, typename = std::enable_if_t<!std::is_integral_v<InputIt>>>
	constexpr static_vector(InputIt first, InputIt last) : static_vector() {
		base_t::assign(first, last);
	}
	
	constexpr static_vector(size_type count, const value_type& value) : static_vector() {
		base_t::assign(count, value);
	}
	
	constexpr static_vector(std::initializer_list<value_type> init) : static_vector(init.begin(), init.end()) {}
	
	constexpr static_vector(const static_vector& other) : static_vector(other.begin(), other.end()) {}
	
	constexpr static_vector(static_vector&& other) noexcept(std::is_nothrow_swappable_v<T>) : static_vector() {
		swap(other);
	}
	
	~static_vector() noexcept(std::is_nothrow_destructible_v<T>) {}

	// operations

	constexpr auto& operator=(static_vector&& other) noexcept(std::is_nothrow_swappable_v<T>) {
		swap(other);
		return *this;
	}

	constexpr auto& operator=(const static_vector& other) {
		base_t::assign(other.begin(), other.end());
		return *this;
	}

	constexpr auto& operator=(std::initializer_list<value_type> ilist) {
		base_t::assign(ilist.begin(), ilist.end());
		return *this;
	}

	// element access

	constexpr auto swap(static_vector& other) noexcept(std::is_nothrow_swappable_v<T>)->void {
		base_t::swap(other);
	}

private:
	alignas(T) std::byte m_storage[Capacity][sizeof(T)];
};

}

template<typename T, std::size_t Capacity>
class std::tuple_size<perfvect::static_vector<T, Capacity>> : public std::integral_constant<std::size_t, Capacity> {};

#endif
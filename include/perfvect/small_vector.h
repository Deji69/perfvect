#ifndef PERFVECT_SMALL_VECTOR_H
#define PERFVECT_SMALL_VECTOR_H

#include "iterator.h"
#include "static_vector.h"
#include "vector.h"
#include <memory>
#include <memory_resource>
#include <type_traits>
#include <utility>

namespace perfvect {

template<
	typename T,
	std::size_t StaticCapacity = 16,
	std::size_t DynamicCapacity = StaticCapacity,
	typename Allocator = std::pmr::polymorphic_allocator<T>
>
class small_vector : public vector<T, Allocator> {
	using base_t = vector<T, Allocator>;
	
public:
	using value_type = typename base_t::value_type;
	using allocator_type = typename base_t::allocator_type;
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
	// constructors
	
	constexpr small_vector() noexcept(std::is_nothrow_constructible_v<T>) :
		base_t(
			reinterpret_cast<pointer>(std::addressof(m_storage[0])),
			StaticCapacity,
			DynamicCapacity
		) {}

	template<typename InputIt, typename = std::enable_if_t<detail::is_iterator_v<InputIt>>>
	constexpr small_vector(InputIt first, InputIt last) : small_vector() {
		base_t::assign(first, last);
	}

	constexpr small_vector(const small_vector& other) : small_vector() {
		base_t::assign(other.begin(), other.end());
	}
	
	constexpr small_vector(small_vector&& other) noexcept(std::is_nothrow_swappable_v<T>) : small_vector() {
		swap(other);
	}
	
	template<typename Alloc = std::allocator<T>>
	constexpr small_vector(const vector<T, Alloc>& other) : small_vector() {
		*this = other;
	}

	template<typename Alloc>
	constexpr small_vector(std::vector<T, Alloc>&& other) noexcept(noexcept(base_t::assign(other))) {
		base_t::assign(other);
	}

	explicit constexpr small_vector(size_type count, const T& value = T()) : small_vector() {
		base_t::assign(count, value);
	}
	
	constexpr small_vector(std::initializer_list<T> init) : small_vector() {
		base_t::assign(init);
	}

	// operations

	constexpr auto& operator=(small_vector&& other) {
		base_t::swap(other);
		return *this;
	}

	constexpr auto& operator=(const small_vector& other) {
		base_t::operator=(other);
		return *this;
	}
	
	template<typename Alloc = std::allocator<T>>
	constexpr auto& operator=(const vector<T, Alloc>& other) {
		base_t::operator=(other);
		return *this;
	}

	constexpr auto& operator=(std::initializer_list<T> ilist) {
		base_t::operator=(ilist);
		return *this;
	}

	// modifiers
	constexpr auto swap(small_vector& other) noexcept(noexcept(base_t::swap(other)))->void {
		base_t::swap(other);
	}

private:
	alignas(T) std::byte m_storage[StaticCapacity][sizeof(T)];
};

}

#endif
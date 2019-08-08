#ifndef PERFVECT_ITERATOR_H
#define PERFVECT_ITERATOR_H

#include <iterator>
#include <type_traits>

namespace perfvect::detail {

template<typename T>
class iterator {
	friend class iterator<std::remove_const_t<T>>;
	friend class iterator<const std::remove_const_t<T>>;

public:
	using iterator_category = std::random_access_iterator_tag;
	using value_type = T;
	using difference_type = ptrdiff_t;
	using pointer = T*;
	using reference = T&;

	constexpr iterator() = default;
	constexpr iterator(const iterator<std::remove_const_t<T>>& iter) : m_ptr(iter.m_ptr) {}
	constexpr explicit iterator(pointer ptr) : m_ptr(ptr) {}

	[[nodiscard]] constexpr auto operator*() const->reference {
		return *operator->();
	}

	[[nodiscard]] constexpr auto operator->() const->pointer {
		return std::launder(m_ptr);
	}

	constexpr auto& operator++() {
		++m_ptr;
		return *this;
	}

	constexpr auto operator++(int) {
		auto tmp = *this;
		++*this;
		return tmp;
	}

	constexpr auto& operator--() {
		--m_ptr;
		return *this;
	}

	constexpr auto operator--(int) {
		auto tmp = *this;
		--*this;
		return tmp;
	}

	constexpr auto& operator+=(const difference_type off) {
		m_ptr += off;
		return *this;
	}

	constexpr auto& operator-=(const difference_type off) {
		m_ptr -= off;
		return *this;
	}

	[[nodiscard]] constexpr auto operator+(const difference_type off) const {
		auto tmp = *this;
		return tmp += off;
	}

	[[nodiscard]] constexpr auto operator-(const difference_type off) const {
		auto tmp = *this;
		return tmp -= off;
	}

	[[nodiscard]] constexpr auto operator-(const iterator& other) const {
		return static_cast<difference_type>(m_ptr - other.m_ptr);
	}

	[[nodiscard]] constexpr auto& operator[](const difference_type off) const {
		return *(*this + off);
	}

	[[nodiscard]] constexpr auto operator==(const iterator& other) const {
		return m_ptr == other.m_ptr;
	}

	[[nodiscard]] constexpr auto operator!=(const iterator& other) const {
		return !(*this == other);
	}

	[[nodiscard]] constexpr auto operator<(const iterator& other) const {
		return m_ptr < other.m_ptr;
	}

	[[nodiscard]] constexpr auto operator>(const iterator& other) const {
		return other < *this;
	}

	[[nodiscard]] constexpr auto operator<=(const iterator& other) const {
		return !(other < *this);
	}

private:
	pointer m_ptr;
};

template<typename T, typename = void>
struct is_iterator {
	static constexpr bool value = false;
};

template<typename T>
struct is_iterator<T, typename std::enable_if_t<!std::is_same<typename std::iterator_traits<T>::value_type, void>::value>> {
	static constexpr bool value = true;
};

template<typename T>
constexpr bool is_iterator_v = is_iterator<T>::value;

}

#endif
#ifndef PERFVECT_ITERATOR_H
#define PERFVECT_ITERATOR_H

#include <iterator>
#include <type_traits>
#include <utility>

namespace perfvect {
namespace detail {

// random access iterator that must satisfy the requirements of LegacyInputIterator

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

	constexpr iterator() noexcept = default;
	constexpr iterator(const iterator<std::remove_const_t<T>>& iter) noexcept : m_ptr(iter.m_ptr) {}
	constexpr explicit iterator(pointer ptr) noexcept : m_ptr(ptr) {}

	constexpr auto& operator=(const iterator& iter) noexcept {
		m_ptr = iter.m_ptr;
		return *this;
	}

	[[nodiscard]] constexpr auto operator*() const noexcept->reference {
		return *operator->();
	}

	[[nodiscard]] constexpr auto operator->() const noexcept->pointer {
		return std::launder(m_ptr);
	}

	constexpr auto& operator++() noexcept {
		++m_ptr;
		return *this;
	}

	constexpr auto operator++(int) noexcept {
		auto tmp = *this;
		++*this;
		return tmp;
	}

	constexpr auto& operator--() noexcept {
		--m_ptr;
		return *this;
	}

	constexpr auto operator--(int) noexcept {
		auto tmp = *this;
		--*this;
		return tmp;
	}

	constexpr auto& operator+=(const difference_type off) noexcept {
		m_ptr += off;
		return *this;
	}

	constexpr auto& operator-=(const difference_type off) noexcept {
		m_ptr -= off;
		return *this;
	}

	[[nodiscard]] constexpr auto operator+(const difference_type off) const noexcept {
		auto tmp = *this;
		return tmp += off;
	}

	[[nodiscard]] constexpr auto operator-(const difference_type off) const noexcept {
		auto tmp = *this;
		return tmp -= off;
	}

	[[nodiscard]] constexpr auto operator-(const iterator& other) const noexcept {
		return static_cast<difference_type>(m_ptr - other.m_ptr);
	}

	[[nodiscard]] constexpr auto& operator[](const difference_type off) const noexcept {
		return *(*this + off);
	}

	[[nodiscard]] constexpr auto operator==(const iterator& other) const noexcept {
		return m_ptr == other.m_ptr;
	}

	[[nodiscard]] constexpr auto operator!=(const iterator& other) const noexcept {
		return !(*this == other);
	}

	[[nodiscard]] constexpr auto operator<(const iterator& other) const noexcept {
		return m_ptr < other.m_ptr;
	}

	[[nodiscard]] constexpr auto operator>(const iterator& other) const noexcept {
		return other < *this;
	}

	[[nodiscard]] constexpr auto operator<=(const iterator& other) const noexcept {
		return !(other < *this);
	}

	auto swap(iterator other) {
		std::swap(m_ptr, other.m_ptr);
	}

private:
	pointer m_ptr = nullptr;
};

template<typename T>
auto swap(iterator<T>& it1, iterator<T>& it2) {
	it1.swap(it2);
}

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
}

template<typename T>
struct std::iterator_traits<perfvect::detail::iterator<T>> {
	using difference_type = typename perfvect::detail::iterator<T>::difference_type;
	using value_type = typename perfvect::detail::iterator<T>::value_type;
	using pointer = typename perfvect::detail::iterator<T>::pointer;
	using reference = typename perfvect::detail::iterator<T>::reference;
	using iterator_category = typename perfvect::detail::iterator<T>::iterator_category;
};

#endif
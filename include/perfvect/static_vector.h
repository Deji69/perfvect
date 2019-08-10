#ifndef PERFVECT_STATIC_VECTOR_H
#define PERFVECT_STATIC_VECTOR_H

#include "iterator.h"
#include <cstddef>
#include <vector>
#include <new>
#include <variant>
#include <type_traits>

#if _DEBUG
#define PERFVECT_NODEBUG_NOEXCEPT
#else
#define PERFVECT_NODEBUG_NOEXCEPT noexcept
#endif

namespace perfvect {

template<typename T, std::size_t Capacity>
class static_vector {
public:
	using value_type = T;
	using size_type = std::size_t;
	using difference_type = std::ptrdiff_t;
	using reference = value_type&;
	using const_reference = const value_type&;
	using pointer = value_type*;
	using const_pointer = const value_type*;
	using iterator = detail::iterator<T>;
	using const_iterator = detail::iterator<const T>;
	using reverse_iterator = std::reverse_iterator<iterator>;
	using const_reverse_iterator = std::reverse_iterator<const_iterator>;

public:
	constexpr static_vector() noexcept : m_data() {}
	constexpr static_vector(static_vector&& other) noexcept(std::is_nothrow_swappable_v<T>) {
		swap(other);
	}
	constexpr static_vector(const static_vector& other) {
		assign(other.begin(), other.end());
	}
	constexpr static_vector(size_type count, const value_type& value) {
		assign(count, value);
	}
	template<typename InputIt, typename = std::enable_if_t<!std::is_integral_v<InputIt>>>
	constexpr static_vector(InputIt first, InputIt last) {
		assign(first, last);
	}
	constexpr static_vector(std::initializer_list<value_type> init) : static_vector(init.begin(), init.end()) {}
	
	~static_vector() noexcept(std::is_trivially_destructible_v<T>) {
		destruct_elements(0);
	}

	// operations

	constexpr auto& operator=(static_vector&& other) noexcept(std::is_nothrow_swappable_v<T>) {
		swap(other);
		return *this;
	}

	constexpr auto& operator=(const static_vector& other) {
		assign(other.begin(), other.end());
		return *this;
	}

	constexpr auto& operator=(std::initializer_list<value_type> ilist) {
		assign(ilist.begin(), ilist.end());
		return *this;
	}

	constexpr auto assign(size_type count, const value_type& value) {
		m_size = count;
		std::fill_n(data(), count, value);
	}

	template<typename InputIt, typename = std::enable_if_t<!std::is_integral_v<InputIt>>>
	constexpr auto assign(InputIt first, InputIt last) {
		auto count = static_cast<size_type>(last - first);
		
		if (count < m_size) {
			std::copy_n(first, count, data());
			destruct_elements(count);
		}
		else if (count == m_size) {
			std::copy_n(first, count, data());
		}
		else if (count > m_size) {
			auto dest = std::copy_n(first, m_size, begin());
			insert(dest, first + m_size, last);
		}

		m_size = count;
	}

	constexpr auto assign(std::initializer_list<value_type> ilist) {
		assign(ilist.begin(), ilist.end());
	}

	constexpr auto fill(const value_type& value) {
		m_size = Capacity;
		std::fill_n(data(), Capacity, value);
	}

	// iterators

	[[nodiscard]] constexpr auto begin() noexcept {
		return iterator(data());
	}

	[[nodiscard]] constexpr auto begin() const noexcept {
		return const_iterator(data());
	}

	[[nodiscard]] constexpr auto end() noexcept {
		return iterator(data() + m_size);
	}

	[[nodiscard]] constexpr auto end() const noexcept {
		return const_iterator(data() + m_size);
	}

	[[nodiscard]] constexpr auto rbegin() noexcept {
		return reverse_iterator(end());
	}

	[[nodiscard]] constexpr auto rbegin() const noexcept {
		return const_reverse_iterator(end());
	}

	[[nodiscard]] constexpr auto rend() noexcept {
		return reverse_iterator(begin());
	}

	[[nodiscard]] constexpr auto rend() const noexcept {
		return const_reverse_iterator(begin());
	}

	[[nodiscard]] constexpr auto cbegin() const noexcept {
		return begin();
	}

	[[nodiscard]] constexpr auto cend() const noexcept {
		return end();
	}

	[[nodiscard]] constexpr auto crbegin() const noexcept {
		return rbegin();
	}

	[[nodiscard]] constexpr auto crend() const noexcept {
		return rend();
	}

	// element access

	[[nodiscard]] constexpr auto at(size_type pos)->reference {
		if (m_size <= pos) {
			throw std::out_of_range("invalid static_vector<T, N> subscript");
		}

		return data()[pos];
	}
	
	[[nodiscard]] constexpr auto at(size_type pos) const->const_reference {
		if (m_size <= pos) {
			throw std::out_of_range("invalid static_vector<T, N> subscript");
		}

		return data()[pos];
	}

	[[nodiscard]] constexpr auto operator[](size_type pos) PERFVECT_NODEBUG_NOEXCEPT->reference {
		#if _DEBUG
			if (pos >= m_size)
				throw std::out_of_range("static_vector subscript out of range");
		#endif

		return data()[pos];
	}

	[[nodiscard]] constexpr auto operator[](size_type pos) const PERFVECT_NODEBUG_NOEXCEPT->const_reference {
		#if _DEBUG
			if (pos >= m_size)
				throw std::out_of_range("static_vector subscript out of range");
		#endif

		return data()[pos];
	}

	[[nodiscard]] constexpr auto front() noexcept->reference {
		return *data();
	}

	[[nodiscard]] constexpr auto front() const noexcept->const_reference {
		return *data();
	}

	[[nodiscard]] constexpr auto back() noexcept->reference {
		return *(data() + (m_size - 1));
	}

	[[nodiscard]] constexpr auto back() const noexcept->const_reference {
		return *(data() + (m_size - 1));
	}

	// Modifying the returned pointer and attempting to access the resulting object
	// is "weirdly UB" - though the committee is looking to define this behaviour in
	// future and there's most likely no actual harm in any existing implementation.
	[[nodiscard]] constexpr auto data() noexcept->pointer {
		return std::launder(get_address(0));
	}

	// Modifying the returned pointer and attempting to access the resulting object
	// is "weirdly UB" - though the committee is looking to define this behaviour in
	// future and there's most likely no actual harm in any existing implementation.
	[[nodiscard]] constexpr auto data() const noexcept->const_pointer {
		return std::launder(get_address(0));
	}

	// capacity

	[[nodiscard]] constexpr auto capacity() const noexcept {
		return Capacity;
	}

	[[nodiscard]] constexpr auto size() const noexcept {
		return m_size;
	}

	[[nodiscard]] constexpr auto max_size() const noexcept {
		return capacity();
	}

	[[nodiscard]] constexpr auto empty() const noexcept {
		return size() == 0;
	}

	// modifiers

	constexpr auto insert(const_iterator pos, const value_type& val)->iterator {
		return emplace(pos, val);
	}

	constexpr auto insert(const_iterator pos, value_type&& val)->iterator {
		return emplace(pos, std::move(val));
	}

	constexpr auto insert(const_iterator pos, const size_type count, const value_type& val) {
		if (!count) return iterator_const_cast(pos);

		auto myfirst = begin();
		const auto off = static_cast<size_type>(pos - begin());
		const auto oldsize = size();

		std::fill(end(), end() + count, val);
		m_size = oldsize + count;

		if (pos != end()) {
			std::rotate(myfirst + off, myfirst + oldsize, end());
		}
		return myfirst + off;
	}

	template<typename Iter, typename = std::enable_if_t<detail::is_iterator_v<Iter>>>
	auto insert(const_iterator pos, Iter first, Iter last)->iterator {
		if (first == last) return iterator_const_cast(pos);
		
		auto myfirst = begin();
		const auto off = static_cast<size_type>(pos - myfirst);
		const auto oldsize = size();

		for (; first != last; ++first) {
			emplace_back(*first);
		}

		std::rotate(myfirst + off, myfirst + oldsize, end());
		return myfirst + off;
	}

	constexpr auto insert(const_iterator pos, std::initializer_list<value_type> list) {
		return insert(pos, list.begin(), list.end());
	}

	template<typename... Args>
	auto emplace(const_iterator pos, Args&&... args)->iterator {
		emplace_back(std::forward<Args>(args)...);
		auto first = iterator_const_cast(pos);
		std::rotate<iterator>(first, iterator(data() + m_size - 1), end());
		return iterator_const_cast(pos);
	}

	constexpr auto erase(const_iterator pos)->iterator {
		auto first = iterator_const_cast(pos);
		std::rotate(first, first + 1, end());
		destruct_elements(m_size - 1);
		--m_size;
		return first + 1;
	}

	constexpr auto erase(const_iterator first, const_iterator last)->const_iterator {
		if (first != last) {
			const auto myfirst = iterator_const_cast(first);
			const auto mylast = iterator_const_cast(last);
			const auto count = std::distance(myfirst, mylast);
			std::rotate(myfirst, mylast, end());
			destruct_elements(m_size - count);
			m_size -= count;
			return myfirst;
		}
		return last;
	}

	template<typename... Args>
	auto& emplace_back(Args&&... args) {
		return *construct_element(m_size++, std::forward<Args>(args)...);
	}

	constexpr auto resize(size_type count) {
		if (count == m_size) return;
		if (count < m_size) {
			erase(begin() + count, end());
		}
		else {
			insert(end(), count - m_size, value_type());
		}
	}

	constexpr auto resize(size_type count, const value_type& value) {
		if (count <= m_size) return resize(count);
		insert(end(), count - m_size, value);
	}

	auto push_back(const value_type& val) {
		emplace_back(val);
	}

	auto push_back(value_type&& val) {
		emplace_back(std::move(val));
	}

	constexpr auto pop_back() {
		#if _DEBUG
		if (empty())
			throw std::out_of_range("static_vector empty on pop_back");
		#endif

		destruct_elements(m_size - 1);
		--m_size;
	}

	constexpr auto clear() {
		destruct_elements(0);
		m_size = 0;
	}

	constexpr auto swap(static_vector& other) noexcept(std::is_nothrow_swappable_v<T>)->void {
		if (this == std::addressof(other)) return;
		auto ptr = data();
		auto otherPtr = other.data();
		for (auto swapSize = std::min(m_size, other.m_size); swapSize; --swapSize) {
			std::swap(*ptr++, *otherPtr++);
		}
		if (other.m_size > m_size) {
			for (auto left = m_size; left < other.m_size; ++left) {
				construct_element(left, std::move(*otherPtr++));
			}

			other.destruct_elements(m_size);
		}
		if (m_size > other.m_size) {
			for (auto left = other.m_size; left < m_size; ++left) {
				other.construct_element(left, std::move(*ptr++));
			}

			destruct_elements(other.m_size);
		}

		std::swap(m_size, other.m_size);
	}

private:
	constexpr auto get_address(size_type idx)->pointer {
		return reinterpret_cast<pointer>(&m_data[idx]);
	}

	constexpr auto get_address(size_type idx) const->const_pointer {
		return reinterpret_cast<const_pointer>(&m_data[idx]);
	}

	constexpr auto destruct_elements(size_type from) {
		auto base = data();
		for (; from != m_size; ++from) {
			(base + from)->~value_type();
		}
	}

	template<typename... Args>
	auto construct_element(size_type pos, Args&&... args) {
		return new (get_address(pos)) value_type{std::forward<Args>(args)...};
	}
	
	auto iterator_const_cast(const_iterator iter)->iterator
	{
		return iterator(const_cast<T*>(&*iter));
	}

private:
	alignas(T) std::byte m_data[Capacity][sizeof(T)];
	std::size_t m_size = 0;
};

}

template<typename T, std::size_t Capacity>
class std::tuple_size<perfvect::static_vector<T, Capacity>> : public std::integral_constant<std::size_t, Capacity> {};

#endif
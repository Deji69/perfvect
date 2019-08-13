#ifndef PERFVECT_SMALL_VECTOR_H
#define PERFVECT_SMALL_VECTOR_H

#include "iterator.h"
#include "static_vector.h"
#include <memory>
#include <type_traits>
#include <variant>
#include <vector>

namespace perfvect {

// small_vector is an std::variant wrapper for static_vector and std::vector.
// It allows interfacing with both vector variants as a single vector
// static_vector will be used until the number of elements exceeds StaticCapacity.
// After that, the std::vector will be used until the number of elements is reduced enough and
// shrink_to_fit() is called.
// Upon switching from vector to dynamic variants, all elements from the static vector are moved
// to the dynamic vector. The dynamic vector starts off with a capacity of DynamicCapacity, or
// higher if more elements are needed.

template<typename T, typename Allocator = std::allocator<T>>
class small_vector_base {
	using StaticVec = static_vector_base<T>;
	using DynamicVec = std::vector<T, Allocator>;
	using Variant = std::variant<StaticVec*, DynamicVec>;
	
public:
	using value_type = T;
	using allocator_type = Allocator;
	using size_type = std::size_t;
	using difference_type = std::ptrdiff_t;
	using reference = value_type&;
	using const_reference = const value_type&;
	using pointer = typename std::allocator_traits<allocator_type>::pointer;
	using const_pointer = typename std::allocator_traits<allocator_type>::const_pointer;
	using iterator = detail::iterator<value_type>;
	using const_iterator = detail::iterator<const value_type>;
	using reverse_iterator = std::reverse_iterator<iterator>;
	using const_reverse_iterator = std::reverse_iterator<const_iterator>;

public:
	// constructors
	
	constexpr small_vector_base(StaticVec& staticStorage, size_type minDynamicCapacity) noexcept :
		m_staticVec(staticStorage), m_minDynamicCapacity(minDynamicCapacity) {}

	explicit constexpr small_vector_base(size_type count, const T& value = T()) {
		assign(count, value);
	}
	
	constexpr small_vector_base(std::initializer_list<T> init) {
		assign(init);
	}

	// operations

	constexpr auto& operator=(small_vector_base&& other) {
		assign(other);
		return *this;
	}

	template<typename Alloc = std::allocator<T>>
	constexpr auto& operator=(const small_vector_base<T, Alloc>& other) {
		assign(other);
		return *this;
	}

	constexpr auto& operator=(std::initializer_list<T> ilist) {
		assign(ilist);
		return *this;
	}

	template<typename Alloc>
	constexpr auto assign(const small_vector_base<T, Alloc>& other) {
		assign_vec(other);
	}

	template<typename InputIt, typename = std::enable_if_t<detail::is_iterator_v<InputIt>>>
	constexpr auto assign(InputIt first, InputIt last) {
		assign_vec(std::distance(first, last), first, last);
	}

	constexpr auto assign(std::initializer_list<T> ilist) {
		assign_vec(ilist.size(), ilist.begin(), ilist.end());
	}

	constexpr auto assign(size_type count, const T& value) {
		assign_vec(count, count, value);
	}

	// iterators

	[[nodiscard]] constexpr auto begin() noexcept->iterator {
		return is_static() ? as_static().begin() : iterator(&*as_dynamic().begin());
	}

	[[nodiscard]] constexpr auto begin() const noexcept->const_iterator {
		return is_static() ? as_static().cbegin() : const_iterator(&*as_dynamic().cbegin());
	}

	[[nodiscard]] constexpr auto end() noexcept->iterator {
		if (is_static()) return as_static().end();
		auto& vec = as_dynamic();
		return iterator(vec.data() + vec.size());
	}

	[[nodiscard]] constexpr auto end() const noexcept->const_iterator {
		if (is_static()) return as_static().cend();
		auto& vec = as_dynamic();
		return const_iterator(vec.data() + vec.size());
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

	[[nodiscard]] constexpr auto cbegin() const noexcept->const_iterator {
		return begin();
	}

	[[nodiscard]] constexpr auto cend() const noexcept->const_iterator {
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
		return is_static() ? as_static().at(pos) : as_dynamic().at(pos);
	}
	
	[[nodiscard]] constexpr auto at(size_type pos) const->const_reference {
		return is_static() ? as_static().at(pos) : as_dynamic().at(pos);
	}

	[[nodiscard]] constexpr auto operator[](size_type pos) noexcept->reference {
		return is_static() ? as_static()[pos] : as_dynamic()[pos];
	}

	[[nodiscard]] constexpr auto operator[](size_type pos) const noexcept->const_reference {
		return is_static() ? as_static()[pos] : as_dynamic()[pos];
	}

	[[nodiscard]] constexpr auto front() noexcept->reference {
		return is_static() ? as_static().front() : as_dynamic().front();
	}

	[[nodiscard]] constexpr auto front() const noexcept->const_reference {
		return is_static() ? as_static().front() : as_dynamic().front();
	}

	[[nodiscard]] constexpr auto back() noexcept->reference {
		return is_static() ? as_static().back() : as_dynamic().back();
	}

	[[nodiscard]] constexpr auto back() const noexcept->const_reference {
		return is_static() ? as_static().back() : as_dynamic().back();
	}

	[[nodiscard]] constexpr auto data() noexcept->pointer {
		return is_static() ? as_static().data() : as_dynamic().data();
	}

	[[nodiscard]] constexpr auto data() const noexcept->const_pointer {
		return is_static() ? as_static().data() : as_dynamic().data();
	}

	// capacity

	[[nodiscard]] constexpr auto capacity() const noexcept {
		return m_isStatic ? m_staticVec.capacity() : m_dynamicVec.capacity();
	}

	[[nodiscard]] constexpr auto size() const noexcept {
		return m_isStatic ? m_staticVec.size() : m_dynamicVec.size();
	}

	[[nodiscard]] constexpr auto empty() const noexcept {
		return m_isStatic ? m_staticVec.empty() : m_dynamicVec.empty();
	}

	[[nodiscard]] constexpr auto is_static() const {
		return m_isStatic;
	}

	[[nodiscard]] constexpr auto is_dynamic() const {
		return !is_static();
	}

	auto reserve(size_type new_cap) {
		if (is_static()) {
			if (new_cap > as_static().capacity()) {
				convert_to_dynamic(new_cap);
			}
		}
		else {
			auto& vec = as_dynamic();
			vec.reserve(new_cap);
		}
	}

	auto shrink_to_fit() {
		if (is_static()) return;
		if (size() < m_staticVec.capacity()) {
			convert_to_static();
		}
		else {
			auto& vec = as_dynamic();
			vec.shrink_to_fit();
		}
	}

	// modifiers

	constexpr auto insert(const_iterator pos, const T& val)->iterator {
		reserve(size() + 1);
		return is_static() ? as_static().insert(pos, val) : as_dynamic().insert(pos, val);
	}

	constexpr auto insert(const_iterator pos, T&& val)->iterator {
		reserve(size() + 1);
		return is_static() ? as_static().insert(pos, val) : as_dynamic().insert(pos, val);
	}

	constexpr auto insert(const_iterator pos, const size_type count, const T& val) {
		reserve(size() + count);
		return is_static() ? as_static().insert(pos, count, val) : as_dynamic().insert(pos, count, val);
	}

	template<typename Iter, typename = std::enable_if_t<detail::is_iterator_v<Iter>>>
	auto insert(const_iterator pos, Iter first, Iter last)->iterator {
		reserve(size() + std::distance(first, last));
		return is_static() ? as_static().insert(pos, first, last) : as_dynamic().insert(pos, first, last);
	}

	constexpr auto insert(const_iterator pos, std::initializer_list<T> list) {
		reserve(size() + list.size());
		return is_static() ? as_static().insert(pos, list) : as_dynamic().insert(pos, list);
	}

	template<typename... Args>
	auto emplace(const_iterator pos, Args&&... args)->iterator {
		reserve(size() + 1);
		if (is_static()) return as_static().insert(pos, std::forward<Args>(args)...);
		return as_dynamic().insert(pos, std::forward<Args>(args)...);
	}

	template<typename... Args>
	auto& emplace_back(Args&&... args) {
		reserve(size() + 1);
		if (is_static()) as_static().emplace_back(std::forward<Args>(args)...);
		return as_dynamic().emplace_back(std::forward<Args>(args)...);
	}

	auto push_back(const T& val) {
		reserve(size() + 1);
		if (is_static()) as_static().push_back(val);
		else as_dynamic().push_back(val);
	}

	auto push_back(T&& val) {
		reserve(size() + 1);
		if (is_static()) as_static().push_back(val);
		else as_dynamic().push_back(val);
	}

	constexpr auto pop_back() {
		if (is_static()) as_static().pop_back();
		else as_dynamic().pop_back();
	}

	constexpr auto erase(const_iterator pos)->iterator {
		return is_static() ? as_static().erase(pos) : as_dynamic().erase(pos);
	}

	constexpr auto erase(const_iterator first, const_iterator last)->const_iterator {
		if (is_static()) {
			auto& vec = as_static();
			auto it = vec.erase(first, last);
			return it;
		}

		auto& vec = as_dynamic();
		auto it = vec.erase(first, last);
		return it;
	}

	constexpr auto resize(size_type count) {
		reserve(count);
		if (is_static()) as_static().resize(count);
		else as_dynamic().resize(count);
	}

	constexpr auto resize(size_type count, const T& value) {
		reserve(count);
		if (is_static()) as_static().resize(count, value);
		else as_dynamic().resize(count, value);
	}

	constexpr auto clear() {
		if (is_static()) as_static().clear();
		else as_dynamic().clear();
	}

protected:
	template<typename Alloc>
	auto assign_vec(const small_vector_base<T, Alloc>& other) {
		assign_vec(other.size(), other.cbegin(), other.cend());
	}

	template<typename... Args>
	auto assign_vec(size_type size, Args&&... args) {
		if (is_dynamic() || size > m_staticVec.capacity())
			assign_dynamic(size, std::forward<Args>(args)...);
		else
			assign_static(std::forward<Args>(args)...);
	}

	template<typename... Args>
	auto& assign_static(Args&&... args) {
		if (!m_dynamicVec.empty()) m_dynamicVec.clear();
		m_staticVec.assign(std::forward<Args>(args)...);
		m_isStatic = true;
		return m_staticVec;
	}

	template<typename... Args>
	auto& assign_dynamic(size_type reserve_size, Args&&... args) {
		if (!m_staticVec.empty()) m_staticVec.clear();
		m_dynamicVec.reserve(reserve_size > m_minDynamicCapacity ? reserve_size : m_minDynamicCapacity);
		m_dynamicVec.assign(std::forward<Args>(args)...);
		m_isStatic = false;
		return m_dynamicVec;
	}

	auto& convert_to_dynamic(size_type reserve_size = 0) {
		auto& myvec = as_static();
		m_dynamicVec.reserve(reserve_size > m_minDynamicCapacity ? reserve_size : m_minDynamicCapacity);
		m_dynamicVec.assign(std::make_move_iterator(myvec.begin()), std::make_move_iterator(myvec.end()));
		m_isStatic = false;
		return m_dynamicVec;
	}

	auto& convert_to_static() noexcept(std::is_nothrow_move_assignable_v<T>) {
		auto& myvec = as_dynamic();
		m_staticVec.assign(std::make_move_iterator(myvec.begin()), std::make_move_iterator(myvec.end()));
		m_isStatic = true;
		return m_staticVec;
	}

	[[nodiscard]] constexpr auto& as_static() noexcept {
		return m_staticVec;
	}

	[[nodiscard]] constexpr auto& as_static() const noexcept {
		return m_staticVec;
	}

	[[nodiscard]] constexpr auto& as_dynamic() noexcept {
		return m_dynamicVec;
	}

	[[nodiscard]] constexpr auto& as_dynamic() const noexcept {
		return m_dynamicVec;
	}

protected:
	DynamicVec m_dynamicVec;
	StaticVec& m_staticVec;
	std::size_t m_minDynamicCapacity = 0;
	bool m_isStatic = true;
};

template<typename T, std::size_t StaticCapacity = 16, std::size_t DynamicCapacity = 32, typename Allocator = std::allocator<T>>
class small_vector : public small_vector_base<T, Allocator> {
	using base_t = small_vector_base<T, Allocator>;
	using StaticVec = static_vector<T, StaticCapacity>;
	using DynamicVec = std::vector<T, Allocator>;
	using Variant = std::variant<StaticVec, DynamicVec>;
	
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
	
	constexpr small_vector() noexcept(std::is_nothrow_constructible_v<T>)
		: base_t(m_staticStorage, DynamicCapacity) {}

	template<typename InputIt, typename = std::enable_if_t<detail::is_iterator_v<InputIt>>>
	constexpr small_vector(InputIt first, InputIt last) : small_vector() {
		base_t::assign(first, last);
	}

	constexpr small_vector(const small_vector& other) : small_vector() {
		base_t::assign_vec(other);
	}
	
	constexpr small_vector(small_vector&& other) noexcept(std::is_nothrow_swappable_v<T>) : small_vector() {
		swap(other);
	}
	
	template<std::size_t OtherStaticCapacity, std::size_t OtherDynamicCapacity, typename Alloc = std::allocator<T>>
	constexpr small_vector(const small_vector<T, OtherStaticCapacity, OtherDynamicCapacity, Alloc>& other) : small_vector() {
		base_t::assign(other);
	}

	template<typename Alloc>
	constexpr small_vector(std::vector<T, Alloc>&& other) noexcept(std::is_nothrow_move_constructible_v<T>) {
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
		base_t::assign(other);
		return *this;
	}

	constexpr auto& operator=(const small_vector& other) {
		base_t::assign(other);
		return *this;
	}

	constexpr auto& operator=(std::initializer_list<T> ilist) {
		base_t::assign(ilist);
		return *this;
	}

	// modifiers
	constexpr auto swap(small_vector& other)->void {
		base_t::m_staticVec.swap(other.m_staticVec);
		base_t::m_dynamicVec.swap(other.m_dynamicVec);
		std::swap(base_t::m_isStatic, other.m_isStatic);
	}

private:
	StaticVec m_staticStorage;
};

}

#endif
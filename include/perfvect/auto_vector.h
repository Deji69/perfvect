#ifndef PERFVECT_AUTO_VECTOR_H
#define PERFVECT_AUTO_VECTOR_H

#include "iterator.h"
#include "static_vector.h"
#include <memory>
#include <type_traits>
#include <variant>
#include <vector>

namespace perfvect {

// auto_vector is an std::variant wrapper for two types of vector, one static and one dynamic.
// The static vector must satisfy at least the requirements of std::array.
// The dynamic vector must satisfy at least the requirements of std::vector.
// The static vector will be used until the number of elements exceeds StaticVec().max_size().
// After that, the dynamic vector will be used until the number of elements is reduced enough
// and shrink_to_fit() is called.
// Upon switching from vector to dynamic variants, all elements from the static vector are
// moved to the dynamic vector. The dynamic vector starts off with a capacity of DynamicCapacity.
//
// Template parameters:
//   StaticVec = the static vector type
//   DynamicVec = the dynamic vector type
//   StaticCapacity = how much static/stack space to use before dynamically allocating
//   DynamicCapacity = if a dynamic allocation has to be made, reserve space for at least this many elements

template<typename StaticVec, typename DynamicVec, std::size_t DynamicCapacity = 64>
class auto_vector {
	constexpr static auto StaticCapacity = std::tuple_size_v<StaticVec>;

	using variant = std::variant<StaticVec, DynamicVec>;

	static_assert(std::is_same_v<typename StaticVec::value_type, typename DynamicVec::value_type>, "value_type of StaticVec and DynamicVec must be the same");
	
public:
	using value_type = typename StaticVec::value_type;
	using allocator_type = typename DynamicVec::allocator_type;
	using size_type = typename DynamicVec::size_type;
	using difference_type = typename DynamicVec::difference_type;
	using reference = value_type&;
	using const_reference = const value_type&;
	using pointer = typename std::allocator_traits<allocator_type>::pointer;
	using const_pointer = typename std::allocator_traits<allocator_type>::const_pointer;
	using iterator = typename detail::iterator<value_type>;
	using const_iterator = typename detail::iterator<const value_type>;
	using reverse_iterator = std::reverse_iterator<iterator>;
	using const_reverse_iterator = std::reverse_iterator<const_iterator>;

public:
	constexpr auto_vector() noexcept(std::is_nothrow_constructible_v<value_type>) :
		m_data(std::in_place_type_t<StaticVec>{}) {}

	constexpr auto_vector(const auto_vector& other) {
		assign_vec(other);
	}
	
	template<typename OtherStaticVec, typename OtherDynamicVec, std::size_t OtherDynamicCapacity>
	constexpr auto_vector(const auto_vector<OtherStaticVec, OtherDynamicVec, OtherDynamicCapacity>& other) {
		assign(other);
	}
	
	constexpr auto_vector(auto_vector&& other) noexcept : m_data(std::move(other.m_data)) {}

	constexpr auto_vector(DynamicVec&& other) noexcept : m_data(other) {}

	template<typename InputIt, typename = std::enable_if_t<detail::is_iterator_v<InputIt>>>
	constexpr auto_vector(InputIt first, InputIt last) {
		assign(first, last);
	}

	explicit constexpr auto_vector(size_type count, const value_type& value = value_type()) {
		assign(count, value);
	}
	
	constexpr auto_vector(std::initializer_list<value_type> init) {
		assign(init);
	}

	// operations

	constexpr auto& operator=(auto_vector&& other) {
		assign(other);
		return *this;
	}

	constexpr auto& operator=(const auto_vector& other) {
		assign(other);
		return *this;
	}

	constexpr auto& operator=(std::initializer_list<value_type> ilist) {
		assign(ilist);
		return *this;
	}

	template<typename InputIt, typename = std::enable_if_t<detail::is_iterator_v<InputIt>>>
	constexpr auto assign(InputIt first, InputIt last) {
		assign_vec(std::distance(first, last), first, last);
	}

	constexpr auto assign(std::initializer_list<value_type> ilist) {
		assign_vec(ilist.size(), ilist.begin(), ilist.end());
	}

	constexpr auto assign(size_type count, const value_type& value) {
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
		if (is_static()) return as_static.end();
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

	[[nodiscard]] constexpr auto operator[](size_type pos) PERFVECT_NODEBUG_NOEXCEPT->reference {
		return is_static() ? as_static()[pos] : as_dynamic()[pos];
	}

	[[nodiscard]] constexpr auto operator[](size_type pos) const PERFVECT_NODEBUG_NOEXCEPT->const_reference {
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
		return is_static() ? StaticCapacity : as_dynamic().capacity();
	}

	[[nodiscard]] constexpr auto size() const noexcept {
		return is_static() ? as_static().size() : as_dynamic().size();
	}

	[[nodiscard]] constexpr auto empty() const noexcept {
		return is_static() ? as_static().empty() : as_dynamic().empty();
	}

	[[nodiscard]] constexpr auto is_static() const {
		return std::holds_alternative<StaticVec>(m_data);
	}

	auto reserve(size_type new_cap) {
		if (is_static() && new_cap > StaticCapacity) {
			convert_to_dynamic(new_cap);
		}
	}

	auto shrink_to_fit() {
		if (!is_static()) {
			if (size() < StaticCapacity) {
				convert_to_static();
			}
			else {
				get_dynamic().shrink_to_fit();
			}
		}
	}

	// modifiers

	constexpr auto insert(const_iterator pos, const value_type& val)->iterator {
		reserve(size() + 1);
		return is_static() ? as_static().insert(pos, val) : as_dynamic().insert(pos, val);
	}

	constexpr auto insert(const_iterator pos, value_type&& val)->iterator {
		reserve(size() + 1);
		return is_static() ? as_static().insert(pos, val) : as_dynamic().insert(pos, val);
	}

	constexpr auto insert(const_iterator pos, const size_type count, const value_type& val) {
		reserve(size() + count);
		return is_static() ? as_static().insert(pos, count, val) : as_dynamic().insert(pos, count, val);
	}

	template<typename Iter, typename = std::enable_if_t<detail::is_iterator_v<Iter>>>
	auto insert(const_iterator pos, Iter first, Iter last)->iterator {
		reserve(size() + std::distance(first, last));
		return is_static() ? as_static().insert(pos, first, last) : as_dynamic().insert(pos, first, last);
	}

	constexpr auto insert(const_iterator pos, std::initializer_list<value_type> list) {
		reserve(size() + list.size());
		return is_static() ? as_static().insert(pos, list) : as_dynamic().insert(pos, list);
	}

	template<typename... Args>
	auto emplace(const_iterator pos, Args&&... args)->iterator {
		reserve(size() + 1);
		return is_static()
			? as_static().insert(pos, std::forward<Args>(args)...)
			: as_dynamic().insert(pos, std::forward<Args>(args)...);
	}

	template<typename... Args>
	auto& emplace_back(Args&&... args) {
		reserve(size() + 1);
		return is_static()
			? as_static().emplace_back(std::forward<Args>(args)...)
			: as_dynamic().emplace_back(std::forward<Args>(args)...);
	}

	auto push_back(const value_type& val) {
		reserve(size() + 1);
		if (is_static()) as_static().push_back(val);
		else as_dynamic().push_back(val);
	}

	auto push_back(value_type&& val) {
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
		return is_static() ? as_static().erase(first, last) : as_dynamic().erase(first, last);
	}

	constexpr auto resize(size_type count) {
		reserve(count);
		if (is_static()) as_static().resize(count);
		else as_dynamic().resize(count);
	}

	constexpr auto resize(size_type count, const value_type& value) {
		reserve(count);
		if (is_static()) as_static().resize(count, value);
		else as_dynamic().resize(count, value);
	}

	constexpr auto clear() {
		if (is_static()) as_static().clear();
		else as_dynamic().clear();
	}

	constexpr auto swap(auto_vector& other) noexcept(noexcept(m_data.swap(other.m_data)))->void {
		m_data.swap(other.m_data);
	}

private:
	template<typename OtherStaticVec, typename OtherDynamicVec, std::size_t OtherDynamicCapacity>
	auto assign_vec(const auto_vector<OtherStaticVec, OtherDynamicVec, OtherDynamicCapacity>& other) {
		assign_vec(other.size(), other.cbegin(), other.cend());
	}

	template<typename... Args>
	auto assign_vec(size_type size, Args&&... args) {
		if (size > StaticCapacity)
			assign_dynamic(size, std::forward<Args>(args)...);
		else
			assign_static(std::forward<Args>(args)...);
	}

	template<typename... Args>
	auto& assign_static(Args&&... args) {
		auto& vec = m_data.template emplace<StaticVec>();
		vec.assign(std::forward<Args>(args)...);
		return vec;
	}

	template<typename... Args>
	auto& assign_dynamic(size_type reserve_size = 0, Args&&... args) {
		auto& vec = m_data.template emplace<DynamicVec>();
		vec.reserve(reserve_size > DynamicCapacity ? reserve_size : DynamicCapacity);
		vec.assign(std::forward<Args>(args)...);
		return vec;
	}

	auto& convert_to_dynamic(size_type reserve_size = 0) {
		auto& myvec = as_static();
		auto vec = DynamicVec();
		vec.reserve(reserve_size > DynamicCapacity ? reserve_size : DynamicCapacity);
		vec.assign(std::make_move_iterator(myvec.begin()), std::make_move_iterator(myvec.end()));
		return m_data.template emplace<DynamicVec>(std::move(vec));
	}

	auto& convert_to_static() noexcept(std::is_nothrow_move_assignable_v<value_type>) {
		auto& myvec = as_dynamic();
		auto vec = StaticVec();
		vec.assign(std::make_move_iterator(myvec.begin()), std::make_move_iterator(myvec.end()));
		return m_data.template emplace<StaticVec>(std::move(vec));
	}

	[[nodiscard]] constexpr auto& as_static() noexcept {
		return std::get<StaticVec>(m_data);
	}

	[[nodiscard]] constexpr auto& as_static() const noexcept {
		return std::get<StaticVec>(m_data);
	}

	[[nodiscard]] constexpr auto& as_dynamic() noexcept {
		return std::get<DynamicVec>(m_data);
	}

	[[nodiscard]] constexpr auto& as_dynamic() const noexcept {
		return std::get<DynamicVec>(m_data);
	}

private:
	variant m_data;
};

}

#endif
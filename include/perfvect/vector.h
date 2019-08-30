#ifndef PERFVECT_VECTOR_BASE_H
#define PERFVECT_VECTOR_BASE_H
#include "iterator.h"
#include <algorithm>
#include <memory>
#include <memory_resource>
#include <variant>

namespace perfvect {
namespace detail {
	template<typename Allocator>
	struct vector_dynamic_allocator {
		Allocator allocator;
	};
	
	template<>
	struct vector_dynamic_allocator<void> {};

	template<typename Allocator>
	struct vector_small_allocator {
		Allocator allocator;
		typename Allocator::value_type* staticStorage = nullptr;
		std::size_t staticCap = 0;
		std::size_t dynamicMinCap = 0;
	};
}

template<typename T>
class static_vector_base {
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

protected:
	constexpr static_vector_base(pointer data, std::size_t capacity) noexcept
		: m_data(data), m_capacity(capacity) {}

	~static_vector_base() noexcept(std::is_nothrow_destructible_v<T>) {
		destroy_lazily();
	}

public:
	// operations

	constexpr auto& operator=(static_vector_base&& other) noexcept(std::is_nothrow_swappable_v<T>) {
		swap(other);
		return *this;
	}

	constexpr auto& operator=(const static_vector_base& other) {
		assign(other.begin(), other.end());
		return *this;
	}

	constexpr auto& operator=(std::initializer_list<value_type> ilist) {
		assign(ilist.begin(), ilist.end());
		return *this;
	}

	constexpr auto assign(size_type count, const value_type& value) {
		if (m_size > count) destroy(count);
		std::fill_n(data(), m_size, value);
		if (m_size < count) insert(end(), count - m_size, value);
	}

	template<typename InputIt, typename = std::enable_if_t<detail::is_iterator_v<InputIt>>>
	constexpr auto assign(InputIt first, InputIt last) {
		auto count = static_cast<size_type>(last - first);
		assign_hint(count, first, last);
	}

	constexpr auto assign(std::initializer_list<value_type> ilist) {
		assign_hint(ilist.size(), ilist.begin(), ilist.end());
	}

	constexpr auto fill(const value_type& value) {
		assign(m_capacity, value);
	}

	// element access

	[[nodiscard]] constexpr auto at(size_type pos)->reference {
		if (m_size <= pos) {
			throw std::out_of_range("invalid vector_base<T> subscript");
		}
		return data()[pos];
	}

	[[nodiscard]] constexpr auto at(size_type pos) const->const_reference {
		if (m_size <= pos) {
			throw std::out_of_range("invalid vector_base<T> subscript");
		}
		return data()[pos];
	}

	[[nodiscard]] constexpr auto operator[](size_type pos) noexcept->reference {
		check_range_error(pos);
		return data()[pos];
	}

	[[nodiscard]] constexpr auto operator[](size_type pos) const noexcept->const_reference {
		check_range_error(pos);
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

	[[nodiscard]] constexpr auto data() noexcept->pointer {
		return get_address(0);
	}

	[[nodiscard]] constexpr auto data() const noexcept->const_pointer {
		return get_address(0);
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

	// capacity

	[[nodiscard]] constexpr auto capacity() const noexcept {
		return m_capacity;
	}

	[[nodiscard]] constexpr auto size() const noexcept {
		return m_size;
	}

	[[nodiscard]] constexpr auto empty() const noexcept {
		return size() == 0;
	}

	// modifiers

	constexpr auto clear() {
		destroy();
	}

	constexpr auto pop_back() {
		#if _DEBUG
		if (empty())
			throw std::out_of_range("vector_base empty on pop_back");
		#endif

		destroy(m_size - 1);
	}

	constexpr auto erase(const_iterator pos)->iterator {
		const auto first = iterator_const_cast(pos);
		std::rotate(first, first + 1, end());
		destroy(m_size - 1);
		return first + 1;
	}

	constexpr auto erase(const_iterator first, const_iterator last)->const_iterator {
		if (first != last) {
			const auto count = std::distance(first, last);
			std::rotate(iterator_const_cast(first), iterator_const_cast(last), end());
			destroy(m_size - count);
			return first;
		}
		return last;
	}

	template<typename... Args>
	auto& emplace_back(Args&&... args) {
		return *construct(m_size++, std::forward<Args>(args)...);
	}

	auto push_back(const value_type& val) {
		emplace_back(val);
	}

	auto push_back(value_type&& val) {
		emplace_back(std::move(val));
	}

	template<typename... Args>
	auto emplace(const_iterator pos, Args&& ... args)->iterator {
		emplace_back(std::forward<Args>(args)...);
		return rotate_inserted_to(1, pos);
	}

	constexpr auto insert(const_iterator pos, const value_type& val)->iterator {
		return emplace(pos, val);
	}

	constexpr auto insert(const_iterator pos, value_type&& val)->iterator {
		return emplace(pos, std::move(val));
	}

	constexpr auto insert(const_iterator pos, const size_type count, const value_type& val) {
		return insert_at_hint(count, pos, val);
	}

	template<typename Iter, typename = std::enable_if_t<detail::is_iterator_v<Iter>>>
	auto insert(const_iterator pos, Iter first, Iter last)->iterator {
		return insert_at_hint(std::distance(first, last), pos, first, last);
	}

	constexpr auto insert(const_iterator pos, std::initializer_list<value_type> list) {
		return insert_at_hint(list.size(), pos, list.begin(), list.end());
	}

	constexpr auto resize(size_type count) {
		if (count == m_size) return;
		if (count < m_size) erase(begin() + count, end());
		else insert(end(), count - m_size, value_type());
	}

	constexpr auto resize(size_type count, const value_type& value) {
		if (count <= m_size) return resize(count);
		reallocate_at_least(count);
		insert(end(), count - m_size, value);
	}
	
protected:
	constexpr auto swap(static_vector_base& other) noexcept(std::is_nothrow_swappable_v<T>) {
		if (this == std::addressof(other)) return;

		auto ptr = this->m_data;
		auto otherPtr = other.m_data;

		for (auto swapSize = std::min(this->m_size, other.m_size); swapSize; --swapSize) {
			std::swap(*ptr++, *otherPtr++);
		}

		if (other.m_size > this->m_size) {
			for (auto left = this->m_size; left < other.m_size; ++left) {
				this->construct(left, std::move(*otherPtr++));
			}

			other.destroy_lazily(this->m_size);
		}

		if (this->m_size > other.m_size) {
			for (auto left = other.m_size; left < this->m_size; ++left) {
				other.construct(left, std::move(*ptr++));
			}

			this->destroy_lazily(other.m_size);
		}

		std::swap(this->m_size, other.m_size);
	}

protected:
	template<typename InputIt, typename = std::enable_if_t<detail::is_iterator_v<InputIt>>>
	constexpr auto assign_hint(const size_type count, const InputIt first, const InputIt last) {
		if (count < m_size) {
			std::copy_n(first, count, data());
			destroy(count);
		}
		else {
			auto dest = std::copy_n(first, m_size, begin());

			if (count > m_size) {
				insert(dest, first + m_size, last);
			}
		}
	}

	auto assign_hint(const size_type count, const value_type& val) {
		if (m_size > count) destroy(count);
		std::uninitialized_fill_n(begin(), m_size, val);
		if (m_size < count) insert_hint(count - m_size, val);
	}

	template<typename Iter, typename = std::enable_if_t<detail::is_iterator_v<Iter>>>
	auto insert_hint(const size_type count, const Iter first, const Iter last) {
		std::uninitialized_copy(first, last, end());
		m_size += count;
	}

	auto insert_hint(const size_type count, const value_type& val) {
		std::uninitialized_fill_n(end(), count, val);
		m_size += count;
	}

	template<typename Iter, typename = std::enable_if_t<detail::is_iterator_v<Iter>>>
	auto insert_at_hint(const size_type count, const const_iterator pos, const Iter first, const Iter last)->iterator {
		if (!count) return iterator_const_cast(pos);
		insert_hint(count, first, last);
		return rotate_inserted_to(count, pos);
	}

	auto insert_at_hint(const size_type count, const const_iterator pos, const value_type& val)->iterator {
		if (!count) return iterator_const_cast(pos);
		insert_hint(count, val);
		return rotate_inserted_to(count, pos);
	}

	auto rotate_inserted_to(const size_type count, const const_iterator to) {
		return std::rotate(iterator_const_cast(to), end() - count, end()) - count;
	}

	constexpr auto get_address(const size_type idx)->pointer {
		return std::launder(reinterpret_cast<pointer>(std::addressof(m_data[idx])));
	}

	constexpr auto get_address(const size_type idx) const->const_pointer {
		return std::launder(reinterpret_cast<const_pointer>(std::addressof(m_data[idx])));
	}

	template<typename... Args>
	auto construct(size_type pos, Args&& ... args) {
		return new (get_address(pos)) value_type{std::forward<Args>(args)...};
	}

	constexpr auto destroy_lazily(const size_t from = 0) {
		std::destroy_n(m_data + from, m_size - from);
	}

	constexpr auto destroy(const size_t from = 0) {
		destroy_lazily(from);
		m_size = from;
	}

	auto iterator_offset(const_iterator it) {
		return std::distance(cbegin(), it);
	}
	
	auto iterator_const_cast(const_iterator iter)->iterator {
		return iterator(const_cast<T*>(&*iter));
	}

	auto check_range_error(const size_type pos) const {
		#if _DEBUG
		if (pos >= m_size) {
			throw std::out_of_range("vector_base subscript out of range");
		}
		#endif
		(void)pos;
	}

protected:
	pointer m_data = nullptr;
	size_type m_capacity = 0;
	size_type m_size = 0;
};

template<typename T, typename Allocator = std::pmr::polymorphic_allocator<T>>
class vector : public static_vector_base<T> {
	using base_t = static_vector_base<T>;
	constexpr static bool is_dynamic_alloc = !std::is_void_v<Allocator>;

public:
	using value_type = T;
	using allocator_type = Allocator;
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

protected:
	constexpr vector(pointer staticData, size_type staticCap, size_type dynamicMinCap) :
		base_t(staticData, staticCap), m_alloc{{}, staticData, staticCap, dynamicMinCap} {}

	constexpr vector(Allocator&& alloc, pointer staticData = nullptr, size_type staticCap = 0,
		             size_type dynamicMinCap = 0) :
		base_t(staticData, staticCap), m_alloc{alloc, staticData, staticCap, dynamicMinCap} {}

public:
	constexpr vector() noexcept : base_t(nullptr, 0) {};

	explicit vector(const Allocator& alloc) noexcept : vector() {
		m_alloc = alloc;
	}

	template<typename InputIt, typename = std::enable_if_t<detail::is_iterator_v<InputIt>>>
	constexpr vector(InputIt first, InputIt last) : vector() {
		assign(first, last);
	}

	constexpr vector(size_type count, const value_type& value) : vector() {
		assign(count, value);
	}

	constexpr vector(std::initializer_list<value_type> init) : vector() {
		assign(init);
	}

	template<typename Alloc = Allocator>
	constexpr vector(const vector<T, Alloc>& other) : vector(other.begin(), other.end()) {}

	template<typename Alloc = Allocator>
	constexpr vector(vector<T, Alloc>&& other) noexcept(noexcept(swap(other))) : vector() {
		swap(other);
	}
	
	~vector() noexcept(std::is_nothrow_destructible_v<T>) {
		this->destroy_lazily();
	}

public:
	// operations

	template<typename Alloc = Allocator>
	constexpr auto& operator=(vector<T, Alloc>&& other) noexcept(noexcept(swap(other))) {
		swap(other);
		return *this;
	}

	template<typename Alloc = Allocator>
	constexpr auto& operator=(const vector<T, Alloc>& other) {
		assign(other.begin(), other.end());
		return *this;
	}

	constexpr auto& operator=(std::initializer_list<value_type> ilist) {
		assign(ilist.begin(), ilist.end());
		return *this;
	}

	constexpr auto assign(size_type count, const value_type& value) {
		reallocate_at_least(count);
		base_t::assign_hint(count, value);
	}

	template<typename InputIt, typename = std::enable_if_t<detail::is_iterator_v<InputIt>>>
	constexpr auto assign(InputIt first, InputIt last) {
		const auto count = static_cast<size_type>(std::distance(first, last));
		reallocate_at_least(count);
		base_t::assign_hint(count, first, last);
	}

	constexpr auto assign(std::initializer_list<value_type> ilist) {
		const auto count = static_cast<size_type>(ilist.size());
		reallocate_at_least(count);
		base_t::assign_hint(count, ilist.begin(), ilist.end());
	}

	// capacity

	constexpr auto is_static() const noexcept->bool {
		return this->m_data == m_alloc.staticStorage;
	}

	constexpr auto is_dynamic() const noexcept->bool {
		return !is_static();
	}

	[[nodiscard]] constexpr auto max_size() const noexcept->size_type {
		return std::numeric_limits<difference_type>::max();
	}

	template<typename = std::enable_if_t<is_dynamic_alloc>>
	auto reserve(size_type new_cap) {
		if (new_cap > this->m_capacity) {
			if (new_cap > max_size()) throw std::length_error("vector<T> too long");
			reallocate(new_cap);
		}
	}

	template<typename = std::enable_if_t<is_dynamic_alloc>>
	auto shrink_to_fit() {
		if (this->m_capacity) {
			if (this->m_size) reallocate(this->m_size);
			else deallocate();
		}
	}

	// modifiers

	constexpr auto insert(const_iterator pos, const value_type& val)->iterator {
		return emplace(pos, val);
	}

	constexpr auto insert(const_iterator pos, value_type&& val)->iterator {
		return emplace(pos, std::move(val));
	}

	constexpr auto insert(const_iterator pos, const size_type count, const value_type& val) {
		const auto offset = this->iterator_offset(pos);
		prepare_for_growth(count);
		return base_t::insert_at_hint(count, this->cbegin() + offset, val);
	}

	template<typename Iter, typename = std::enable_if_t<detail::is_iterator_v<Iter>>>
	auto insert(const_iterator pos, Iter first, Iter last)->iterator {
		const auto count = static_cast<size_type>(std::distance(first, last));
		const auto offset = this->iterator_offset(pos);
		prepare_for_growth(count);
		return base_t::insert_at_hint(count, this->cbegin() + offset, first, last);
	}

	constexpr auto insert(const_iterator pos, std::initializer_list<value_type> list) {
		const auto offset = this->iterator_offset(pos);
		prepare_for_growth(list.size());
		return base_t::insert_at_hint(list.size(), this->cbegin() + offset, list.begin(), list.end());
	}

	template<typename... Args>
	auto emplace(const_iterator pos, Args&&... args)->iterator {
		const auto offset = this->iterator_offset(pos);
		prepare_for_growth(1);
		return base_t::emplace(this->cbegin() + offset, std::forward<Args>(args)...);
	}

	template<typename... Args>
	auto& emplace_back(Args&&... args) {
		prepare_for_growth(1);
		return base_t::emplace_back(std::forward<Args>(args)...);
	}

	constexpr auto resize(size_type count) {
		reallocate_at_least(count);
		return base_t::resize(count);
	}

	constexpr auto resize(size_type count, const value_type& value) {
		reallocate_at_least(count);
		return base_t::resize(count, value);
	}

	auto push_back(const value_type& val) {
		emplace_back(val);
	}

	auto push_back(value_type&& val) {
		emplace_back(std::move(val));
	}

	constexpr auto swap(vector& other) {
		if (is_dynamic() && other.is_dynamic()) {
			std::swap(this->m_data, other.m_data);
			std::swap(this->m_size, other.m_size);
			std::swap(this->m_capacity, other.m_capacity);
		}
		else {
			if (this == std::addressof(other)) return;
			if (this->size() == other.size()) return base_t::swap(other);
			
			const auto smaller = this->size() < other.size();
			auto& small = smaller ? *this : other;
			auto& big = smaller ? other : *this;
			const auto small_size = small.size();

			std::swap_ranges(small.begin(), small.end(), big.begin());
			small.insert(
				small.end(),
				std::make_move_iterator(big.begin() + small.size()),
				std::make_move_iterator(big.end())
			);
			big.erase(big.begin() + small_size, big.end());
		}
	}

protected:
	auto deallocate() {
		this->destroy_lazily();
		
		if (!is_static()) {
			m_alloc.allocator.deallocate(this->m_data, this->m_capacity);
			set_alloc(nullptr, 0);
		}
	}
	
	auto prepare_for_growth(const size_type count) {
		auto new_cap = this->m_size + count;
		if (this->m_capacity >= new_cap) return;
		reallocate_at_least(new_cap);
	}

	auto reallocate_at_least(const size_type new_cap) {
		if (new_cap <= this->m_capacity) return;
		const auto calc_cap = this->m_capacity * 2;
		reallocate(calc_cap > new_cap ? calc_cap : new_cap);
	}

	auto reallocate(size_type new_cap) {
		const auto use_static = m_alloc.staticCap >= new_cap;
		if (!use_static) new_cap = std::max(new_cap, m_alloc.dynamicMinCap);
		const auto mem = use_static ? m_alloc.staticStorage : m_alloc.allocator.allocate(new_cap);
		
		if (use_static) {
			new_cap = m_alloc.staticCap;
		}
		
		try {
			if constexpr (std::is_nothrow_move_constructible_v<T> || !std::is_copy_constructible_v<T>)
				std::uninitialized_move_n(this->m_data, this->m_size, mem);
			else
				std::uninitialized_copy_n(this->m_data, this->m_size, mem);
		}
		catch (...) {
			if (!use_static) m_alloc.allocator.deallocate(mem, new_cap);
			throw;
		}
	
		deallocate();
		set_alloc(mem, new_cap);
	}

	auto set_alloc(T* const mem, const size_type cap) {
		this->m_data = mem;
		this->m_capacity = cap;
	}

protected:
	detail::vector_small_allocator<Allocator> m_alloc;
};

}

#endif
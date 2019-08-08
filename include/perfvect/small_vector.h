#ifndef PERFVECT_SMALL_VECTOR_H
#define PERFVECT_SMALL_VECTOR_H

#include "static_vector.h"
#include "auto_vector.h"
#include <vector>
#include <memory_resource>

namespace perfvect {
	template<
		typename T,
		std::size_t StaticCapacity = 16,
		std::size_t DynamicCapacity = StaticCapacity,
		typename Allocator = std::pmr::polymorphic_allocator<T>
	>
	using small_vector = auto_vector<static_vector<T, StaticCapacity>, std::vector<T, Allocator>, DynamicCapacity>;
}

#endif
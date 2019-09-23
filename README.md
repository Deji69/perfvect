# perfvect

A small set of vector containers built for improved performance over `std::vector` while maintaining usability and being free from extra dependencies. The libary is header only and easy to include in any project.

This library contains 2 main container types, `static_vector` and `small_vector`, which inherit from a base `vector` type capable of doing a certain set of operations. These are similar to those seen in the Boost library, minus the dependencies on other Boost types and using the STL more instead.

## Types

### `perfvect::vector<T>`

Essentially a `std::vector` which acts as an underlying base of `small_vector` without requiring awareness of the capacities.

### `perfvect::static_vector<T, Capacity>`

A variably sized array container with a fixed capacity, free of allocations.

### `perfvect::static_vector_base<T>`

A base for `static_vector`, `vector` and `small_vector` that needs no awareness of the total capacity.

### `perfvect::small_vector<T, StaticCapacity = 16, DynamicCapacity = StaticCapacity>`

A vector-interface storing both a `perfvect::static_vector` for static storage and a `std::vector` for dynamic storage. The static storage is used until the number of elements grows above `StaticCapacity`. When the number of elements exceeds that, they are moved to `std::vector` which is given a starting capacity of `DynamicCapacity`.

This vector is optimised for small numbers of elements (within `StaticCapacity`), while still allowing growth beyond that size with fewer allocations, at the slight cost of additional overhead per method call and greater size of the container object itself.

The method `.shrink_to_fit()` can be used to attempt to free any unused capacity. If the `static_vector` is the current active storage, nothing happens. Otherwise, if the `.size()` exceeds `StaticCapacity`, it is implementation defined whether the request is fulfilled (see `std::vector<T, Allocator>::shrink_to_fit`) and may also shrink to a capacity below `DynamicCapacity`. If the size is within `StaticCapacity`, the contents of the `std::vector` are guaranteed to be moved back to the `static_vector` and the `std::vector` memory will be freed.
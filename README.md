# perfvect

A small set of vector containers built for improved performance over `std::vector` while maintaining usability and being free from extra dependencies. The libary is header only and easy to include in any project.

This library contains 2 container types, `static_vector` and `small_vector`. These are similar to those seen in the Boost library, minus the dependencies on other Boost types and using the STL more instead.

## Types

### `perfvect::static_vector<T, Capacity>`

A variably sized array container with a fixed capacity, free of allocations.

### `perfvect::auto_vector<StaticT, DynamicT, DynamicCapacity>`

A wrapper for a `std::variant` capable of holding one of two different vector types, `StaticT` which is treated as a fixed capacity vector, and `DynamicT` which is used once the size exceeds the fixed capacity. The capacity of `StaticT` is deduced via `std::tuple_size_v<StaticT>`, which is specialised for `std::tuple`, `std::array` and `perfvect::static_vector`.

On allocating the `DynamicT` vector, `.reserve` is called with at least `DynamicCapacity` or higher if necessary for additional elements.

### `perfvect::small_vector<T, StaticCapacity = 16, DynamicCapacity = StaticCapacity>`

A vector-interface storing both a `perfvect::static_vector` for static storage and a `std::vector` for dynamic storage. The static storage is used until the number of elements grows above `StaticCapacity`. When the number of elements exceeds that, they are moved to `std::vector` which is given a starting capacity of `DynamicCapacity`.

This vector is optimised for small numbers of elements (within `StaticCapacity`), while still allowing growth beyond that size with fewer allocations, at the slight cost of additional overhead per method call and greater size of the container object itself.

The method `.shrink_to_fit()` can be used to attempt to free any unused capacity. If the `static_vector` is the current active storage, nothing happens. Otherwise, if the `.size()` exceeds `StaticCapacity`, it is implementation defined whether the request is fulfilled (see `std::vector<T, Allocator>::shrink_to_fit`) and may also shrink to a capacity below `DynamicCapacity`. If the size is within `StaticCapacity`, the contents of the `std::vector` are guaranteed to be moved back to the `static_vector` and the `std::vector` memory will be freed.
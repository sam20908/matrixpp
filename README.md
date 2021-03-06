# mpp

A C++20 and later matrix library

[![Build Status](https://dev.azure.com/samestimable2016/mpp/_apis/build/status/sam20908.mpp?branchName=main)](https://dev.azure.com/samestimable2016/mpp/_build/latest?definitionId=3&branchName=main)

#### Tested Compilers:

* GCC 10
* MSVC 19.28

## How to Include:

All you need to do is just add it as a subdirectory and link the target:

```cmake
add_subdirectory("mpp")

target_link_libraries(your_target mpp::mpp)
```

---

## Overview

Here is a _super_ broken down example that showcases the API and functionality mpp offers:

```cpp
#include <mpp/matrix.h>
#include <mpp/algorithms.hpp>
#include <mpp/utility.hpp>

#include <iostream>

int main()
{
  /**
   * Creating matrices
   *
   * Extents determine if a matrix is fixed or (partially) flexible
   *
   * The following conditions will make the matrix static:
   * - Explicitly passing sizes different than mpp::dynamic to the template parameters
   * - Deduction guides with 2D std::array will deduce the extents, e.g.:
   *
   * - std::array<std::array<int, 3>, 2> arr_2d{ { 1, 2, 3 }, { 4, 5, 6 } }
   * - mpp::matrix{ arr_2d }; // Deduces to mpp::matrix<int, 2, 3>
   */
  auto m_fully_static = mpp::matrix<int, 3, 3>{};
  auto m_fully_dynamic = mpp::matrix<int, mpp::dynamic, mpp::dynamic>{};
  auto m_dynamic_rows = mpp::matrix<int, mpp::dynamic, 3>{};
  auto m_dynamic_columns = mpp::matrix<int, 3, mpp::dynamic>{};

  // Initialize using 2D initializer list
  auto m_init_2d_list = mpp::matrix{ { 1, 2, 3 }, { 4, 5, 6 } };
  // Note that deduction guides is used again, but it's deduced as mpp::matrix<int, mpp::dynamic, mpp::dynamic>
  m_init_2d_list.rows(); // 2
  m_init_2d_list.columns(); // 3

  const auto range_2d = std::vector<std::vector<int>>{ { 1, 2, 3 }, { 4, 5, 6 } };
  auto m_2d_range = mpp::matrix{ range_2d }; // Initialize from a 2D range
  m_2d_range.rows(); // 2
  m_2d_range.columns(); // 3

  auto iota = [i = 0] mutable { return i++; };
  auto m_generated = mpp::matrix<int, 2, 3>{ iota }; // Generates values from callable
  // Elements are: 0, 1, 2, 3, 4, 5

  /**
   * Algorithms (note: you can change output matrix type by passing a std::type_identity with desired matrix type as the last argument)
   */
  auto det = mpp::determinant(m_fully_static);
  auto inv = mpp::inverse(m_fully_dynamic);

  auto block_static = mpp::block(m_fully_static, 0, 0, 1, 1, std::type_identity<mpp::matrix<int, 2, 2>>{});
  // mpp::matrix<int, 2, 2> 2x2
  auto block_dyn = mpp::block(m_fully_static, 0, 0, 1, 1);
  // mpp::matrix<int, mpp::dynamic, mpp::dynamic> 2x2

  // LU Decomposition algorithm has the exception where you can customize the matrix type of L and U matrix
  auto test = mpp::matrix<int, 3, 3>{ {1, 2, 3}, {4, 5, 6}, {7, 8, 9} };
  auto [l_matrix, u_matrix] = mpp::lu_decomposition(test, std::type_identity<mpp::matrix<int, 3, 3>>{}, std::type_identity<mpp::matrix<float>>{});
  // l_matrix is mpp::matrix<int, 3, 3> and u_matrix is mpp::matrix<float, mpp::dynamic, mpp::dynamic>

  /**
   * Utilities
   */
  auto type = mpp::type(m_fully_static); // mpp::matrix_type::fully_static
  auto singular = mpp::singular(m_fully_static); // true (because matrices default initialize elements to 0, and matrices with 0's are singular)

  /**
   * Lazy evaluated math expressions
   */
  auto expr_object = m_fully_static * m_fully_static * 2 / 3;
  auto result_at_top_left = expr_object(0, 0);
  auto result = mpp::matrix{ expr_object }; // Force evaluation of entire matrix

  /**
   * Ways of printing matrices
   */
  mpp::print_matrix(result);
  std::cout << result; // It automatically inserts newline

  return 0;
}
```

One of the main things to take away is the concept of **"extents"** for dimensions. Matrices default to "dynamic extents", which means the sizes can be provided at runtime and matrices can be (partially) flexible (e.g. you can resize rows and columns of a "fully dynamic" matrix, but a "dynamic rows" matrix can only be resized in rows).

#### Comparisons

Normally with C++20, we could simply provide a `operator<=>` and let the compiler figure out the approriate operators, but that's not going to work when we want to compare matrices of different extents or different value types because of the different base class types. In addition, comparison CPOs were implemented to cover that gap.

`operator<=>` was also provided if you know you'll compare matrices with the same value type (therefore no need to use `mpp::elements_compare` ).

```cpp
#include <mpp/matrix.hpp>
#include <mpp/utility/comparison.hpp>

int main()
{
  // Size for both of these dynamic matrices are deduced as 2x2
  const auto left = mpp::matrix<int>{ { 1, 2 }, { 3, 4 } };
  const auto right = mpp::matrix<int>{ { 1, 2 }, { 3, 5 } };

  const auto ordering = left <=> right; // std::strong_ordering::less

  /**
   * Comparing dimensions
   */

  const auto [row_order_1, column_order_1] = mpp::size_compare(left, right, true, true); // Compare both rows and columns
  // row_order_1 -> std::partial_ordering::equivalent
  // column_order_1 -> std::partial_ordering::equivalent

  const auto [row_order_2, column_order_2] = mpp::size_compare(left, right, true, false); // Compare only rows
  // row_order_2 -> std::partial_ordering::equivalent
  // column_order_2 -> std::partial_ordering::unordered <- unordered is used to indicate "not compared"

  /**
   * Comparing elements
   *
   * Note that the ordering type may change depending on the spaceship return type for the value types
   */

  const auto ordering_1 = mpp::elements_compare(left, right);
  // ordering_1 -> std::strong_ordering::less

  // 41.F / 99.F is recurring 0.41
  const auto left = mpp::matrix<float>{ { 41.F / 99.F } };
  const auto right = mpp::matrix<float>{ { 41.F / 99.F } };
  const auto ordering_2 = mpp::elements_compare(left, right, mpp::floating_point_compare);
  // mpp::floating_point_compare is exposed to the public as a comparator that handles floating points
  // ordering_2 -> std::partial_ordering::equivalent

  return 0;
}
```

#### Customizations

Customizations of options that affect the library globally can be changed via specializing the `mpp::configuration` struct with `mpp::override` tag. **The only catch is that you have to do it BEFORE including other `mpp` headers.**

```cpp
#include <mpp/utility/configuration.hpp>

#include <array>
#include <vector>

namespace mpp
{
  template<>
  struct configuration<override>
  {
    template<typename Value>
    using allocator = my_custom_allocator<Value>;

    static constexpr std::size_t rows_extent    = 10;
    static constexpr std::size_t columns_extent = 10;

    /**
     * These are the default buffer type aliases the library will use, but you can customize them
     */

    template<typename Value, std::size_t RowsExtent, std::size_t ColumnsExtent, typename>
    using static_buffer = std::array<Value, RowsExtent * ColumnsExtent>; // mpp::matrix<int, 1, 2>

    template<typename Value, std::size_t, std::size_t, typename Alloc>
    using dynamic_buffer = std::vector<Value, Alloc>; // mpp::matrix<int>

    template<typename Value, std::size_t, std::size_t ColumnsExtent, typename Alloc>
    using dynamic_rows_buffer = dynamic_buffer<Value, 1, ColumnsExtent, Alloc>; // mpp::matrix<int, mpp::dynamic, 2>

    template<typename Value, std::size_t RowsExtent, std::size_t, typename Alloc>
    using dynamic_columns_buffer = dynamic_buffer<Value, RowsExtent, 1, Alloc>; // mpp::matrix<int, 1, mpp::dynamic>
  };
} // namespace mpp

#include <mpp/matrix.hpp>

int main()
{
  auto m = mpp::matrix<int>{};
  auto r = m.rows_extent(); // 10
  auto c = m.columns_extent(); // 10

  using alloc = typename mpp::matrix<int, mpp::dynamic, mpp::dynamic>::allocator_type;
  // ^ my_custom_allocator<int>

  return 0;
}
```

Finally, note that **all algorithms and utilities** are _customization point objects_. It means that you can customize them by overloading with `tag_invoke` and it will detect your customization.

```cpp
namespace ns
{
  struct dumb_matrix {};

  [[nodiscard]] auto tag_invoke(mpp::determinant_t, dumb_matrix) -> int
  {
    /**
     * Say that dumb_matrix was a "optimized" data structure and has guarantees.
     * Lets say that dumb_matrix GUARANTEES a determinant of 2000 (inpractical example).
     */
    return 2000;
  }
} // namespace ns

int main()
{
  auto dummy = ns::dumb_matrix{};
  auto det = mpp::determinant(dummy); // 2000

  return 0;
}
```

Learn more about the rationale of using `tag_invoke` in FAQ.

#### Custom Iterators

Custom iterators are used to make navigating matrices easier. They also meet `contiguous_iterator` requirement.

```cpp
#include <mpp/matrix.hpp>

int main()
{
  auto matrix = mpp::matrix<int>{ { 1, 2 }, { 3, 4 } };
  auto begin = matrix.begin();

  // Extra functionalities to navigate through the matrix easier
  // 1. Nagivating by rows
  begin.move_forward_rows(1);
  auto val = *begin; // 3

  begin.move_backward_rows(1);
  ++begin;
  val = *begin; // 2

  // 2. Navigating by pair of offsets
  auto begin_2 = matrix.begin();
  begin_2 += { 1, 1 }; // 4

  begin_2 -= { 1, 0 }; // 2

  auto begin_3 = begin_2 + std::pair{ 0, 0 }; // Both point to the same location

  return 0;
}
```

You can find more APIs that are not mentioned in this README in the (upcoming) documentation.

---

## FAQ

#### Why `tag_invoke` for customization?

`tag_invoke` allows putting all the overloads into a single name, `tag_invoke` . With customization by overload resolution, name clashes are reduced down to bare minimum.

#### Why assertions instead of exceptions?

The operations have mathematical preconditions, which means it's safe to assume that the user knows what they're doing, but assertions can still be triggers in debug mode.

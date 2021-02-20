/**
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at

 *   http://www.apache.org/licenses/LICENSE-2.0

 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

#pragma once

#include <mpp/detail/constraints.hpp>
#include <mpp/detail/expr_base.hpp>
#include <mpp/detail/matrix_iterator.hpp>
#include <mpp/detail/public_tags.hpp>
#include <mpp/detail/utility.hpp>
#include <mpp/utility/traits.hpp>

#include <algorithm>
#include <cstddef>
#include <iterator>
#include <stdexcept>
#include <type_traits>

namespace mpp::detail
{
	/**
	 * Base matrix class to store internal data and define common member functions
	 */
	template<typename Buffer, typename Value, std::size_t RowsExtent, std::size_t ColumnsExtent, typename Allocator>
	class matrix_base :
		public expr_base<matrix_base<Buffer, Value, RowsExtent, ColumnsExtent, Allocator>,
			typename Buffer::value_type,
			RowsExtent,
			ColumnsExtent>,
		public traits<Value>
	{
	protected:
		Buffer _buf; // Don't default initailize because custom types might not be DefaultConstructible
		std::size_t _rows;
		std::size_t _cols;

		// No point adding this to overload resolution since all matrices implement their own default constructor. The
		// only thing we need this for is static matrices
		matrix_base() = default;

		explicit matrix_base(std::size_t rows, std::size_t cols, const Value& value, const Allocator& allocator) :
			_buf{ rows * cols, value, allocator }, // @TODO: ISSUE #20
			_rows{ rows },
			_cols{ cols }
		{
		}

		explicit matrix_base(std::size_t rows, std::size_t cols, identity_matrix_tag, const Allocator& allocator) :
			_buf{ rows * cols, Value{}, allocator }, // @TODO: ISSUE #20
			_rows{ rows },
			_cols{ cols }
		{
			if (rows == 0 || cols == 0)
			{
				throw std::invalid_argument("Identity matrix cannot have a rank of 0!");
			}

			if (rows != cols)
			{
				throw std::invalid_argument("Identity matrix must be square!");
			}

			transform_1d_buf_into_identity<Value>(_buf, rows);
		}

		explicit matrix_base(std::size_t rows, std::size_t cols, const Allocator& allocator) :
			_buf{ allocator }, // @TODO: ISSUE #20
			_rows{ rows },
			_cols{ cols }
		{
		}

		explicit matrix_base(const Allocator& allocator) :
			_buf{ allocator }, // @TODO: ISSUE #20
			_rows{ 0 },
			_cols{ 0 }
		{
		}

		template<typename Matrix>
		explicit matrix_base(Matrix&& matrix, const Allocator& allocator) :
			_buf{ std::forward<Matrix>(matrix)._buf, allocator }, // @TODO: ISSUE #20
			_rows{ std::forward<Matrix>(matrix)._rows },
			_cols{ std::forward<Matrix>(matrix)._cols }
		{
		}

		void init_dimension_with_val_static(const Value& value) // @TODO: ISSUE #20
		{
			_rows = RowsExtent;
			_cols = ColumnsExtent;

			std::ranges::fill(_buf, value);
		}

		template<typename Range2D>
		void init_buf_2d_static(Range2D&& rng_2d, bool check_rng_size) // @TODO: ISSUE #20
		{
			const auto [rng_rows, rng_cols] = range_2d_dimensions(std::forward<Range2D>(rng_2d));

			if (check_rng_size && (rng_rows != RowsExtent || rng_cols != ColumnsExtent))
			{
				throw std::invalid_argument("Extents of static matrix and dimensions of initializer does not match!");
			}

			_rows = RowsExtent;
			_cols = ColumnsExtent;

			// Keeps track of the beginning of the current row in the 1D buffer where it's empty
			auto row_begin = _buf.begin();

			if constexpr (std::is_rvalue_reference_v<decltype(rng_2d)>)
			{
				for (auto&& row : rng_2d)
				{
					std::ranges::move(row, row_begin);
					row_begin += rng_cols;
				}
			}
			else
			{
				for (auto&& row : rng_2d)
				{
					std::ranges::copy(row, row_begin);
					row_begin += rng_cols;
				}
			}
		}

		void init_buf_2d_dynamic_without_check(auto&& rng_2d, std::size_t rows, std::size_t cols) // @TODO: ISSUE #20
		{
			_rows = rows;
			_cols = cols;
			_buf.reserve(rows * cols);

			const auto buf_back_inserter = std::back_inserter(_buf);

			if constexpr (std::is_rvalue_reference_v<decltype(rng_2d)>)
			{
				for (auto&& row : rng_2d)
				{
					std::ranges::move(row, buf_back_inserter);
				}
			}
			else
			{
				for (auto&& row : rng_2d)
				{
					std::ranges::copy(row, buf_back_inserter);
				}
			}
		}

		void init_expr_dynamic_without_check(std::size_t rows, std::size_t cols,
			auto&& expr) // @TODO: ISSUE #20
		{
			_rows = rows;
			_cols = cols;

			_buf.reserve(rows * cols);

			for (auto row = std::size_t{}; row < _rows; ++row)
			{
				for (auto col = std::size_t{}; col < _cols; ++col)
				{
					_buf.push_back(expr(row, col));
				}
			}
		}

		template<typename Callable>
		void init_buf_from_callable_dynamic(std::size_t rows, std::size_t cols, Callable&& callable)
		{
			_rows = rows;
			_cols = cols;

			reserve_1d_buf_if_vector(_buf, rows, cols);

			const auto total_size = rows * cols;
			for (auto idx = std::size_t{}; idx < total_size; ++idx)
			{
				_buf.push_back(std::invoke(std::forward<Callable>(callable)));
			}
		}

	public:
		using buffer_type = Buffer;

		using value_type      = Value;
		using difference_type = typename buffer_type::difference_type;
		using pointer         = typename buffer_type::pointer;
		using const_pointer   = typename buffer_type::const_pointer;
		using iterator        = matrix_iterator<typename Buffer::iterator>;
		using const_iterator  = matrix_iterator<typename Buffer::const_iterator>;

		matrix_base(const matrix_base&) = default; // @TODO: ISSUE #20
		matrix_base(matrix_base&&)      = default; // @TODO: ISSUE #20

		auto operator=(const matrix_base&) -> matrix_base& = default; // @TODO: ISSUE #20
		auto operator=(matrix_base&&) -> matrix_base& = default;      // @TODO: ISSUE #20

		[[nodiscard]] auto data() -> pointer // @TODO: ISSUE #20
		{
			return _buf.data();
		}

		[[nodiscard]] auto data() const -> const_pointer // @TODO: ISSUE #20
		{
			return _buf.data();
		}

		[[nodiscard]] auto begin() -> iterator // @TODO: ISSUE #20
		{
			return iterator(_buf.begin(), _cols);
		}

		[[nodiscard]] auto begin() const -> const_iterator // @TODO: ISSUE #20
		{
			return const_iterator(_buf.cbegin(), _cols);
		}

		[[nodiscard]] auto end() -> iterator // @TODO: ISSUE #20
		{
			return iterator(_buf.end(), _cols);
		}

		[[nodiscard]] auto end() const -> const_iterator // @TODO: ISSUE #20
		{
			return const_iterator(_buf.cend(), _cols);
		}

		[[nodiscard]] auto cbegin() -> const_iterator // @TODO: ISSUE #20
		{
			return const_iterator(_buf.cbegin(), _cols);
		}

		[[nodiscard]] auto cbegin() const -> const_iterator // @TODO: ISSUE #20
		{
			return const_iterator(_buf.cbegin(), _cols);
		}

		[[nodiscard]] auto cend() -> const_iterator // @TODO: ISSUE #20
		{
			return const_iterator(_buf.cend(), _cols);
		}

		[[nodiscard]] auto cend() const -> const_iterator // @TODO: ISSUE #20
		{
			return const_iterator(_buf.cend(), _cols);
		}

		[[nodiscard]] auto at(std::size_t row_idx, std::size_t col_idx) const -> const value_type& // @TODO: ISSUE #20
		{
			if (row_idx >= _rows || col_idx >= _cols)
			{
				throw std::out_of_range("Access out of bounds!");
			}

			return operator()(row_idx, col_idx);
		}

		[[nodiscard]] auto operator()(std::size_t row_idx, std::size_t col_idx) -> value_type& // @TODO: ISSUE #20
		{
			const auto idx = idx_2d_to_1d(_cols, row_idx, col_idx);

			return _buf[idx];
		}

		[[nodiscard]] auto operator()(std::size_t row_idx,
			std::size_t col_idx) const -> const value_type& // @TODO: ISSUE #20
		{
			const auto idx = idx_2d_to_1d(_cols, row_idx, col_idx);

			return _buf[idx];
		}

		[[nodiscard]] auto rows() const -> std::size_t // @TODO: ISSUE #20
		{
			return _rows;
		}

		[[nodiscard]] auto columns() const -> std::size_t // @TODO: ISSUE #20
		{
			return _cols;
		}

		void swap(matrix_base& right) // @TODO: ISSUE #20
		{
			using std::swap;

			swap(_rows, right._rows);
			swap(_cols, right._cols);
			swap(_buf, right._buf);
		}

		friend inline void init_matrix_with_1d_rng(
			matrix_base<Buffer, Value, RowsExtent, ColumnsExtent, Allocator>& base,
			auto&& rng,
			std::size_t rows,
			std::size_t cols) // @TODO: ISSUE #20
		{
			base._rows = rows;
			base._cols = cols;

			reserve_1d_buf_if_vector(base._buf, rows, cols);

			if constexpr (std::is_rvalue_reference_v<decltype(rng)>)
			{
				if constexpr (is_vector<Buffer>::value)
				{
					std::ranges::move(rng, std::back_inserter(base._buf));
				}
				else
				{
					std::ranges::move(rng, base._buf.begin());
				}
			}
			else
			{
				if constexpr (is_vector<Buffer>::value)
				{
					std::ranges::copy(rng, std::back_inserter(base._buf));
				}
				else
				{
					std::ranges::copy(rng, base._buf.begin());
				}
			}
		}
	};
} // namespace mpp::detail
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

#include "constraints.hpp"
#include "expr_base.hpp"
#include "utility.hpp"

#include <algorithm>
#include <cstddef>
#include <iterator>
#include <span>
#include <type_traits>

namespace matrixpp::detail
{
	/**
     * Base matrix class to store internal data and define common member functions
     */
	template<typename Buffer, typename Value, std::size_t RowsExtent, std::size_t ColumnsExtent>
	class matrix_base :
		public expr_base<matrix_base<Buffer, Value, RowsExtent, ColumnsExtent>,
			typename Buffer::value_type,
			RowsExtent,
			ColumnsExtent>
	{
	protected:
		Buffer _buf;
		std::size_t _rows{ RowsExtent == std::dynamic_extent ? 0 : RowsExtent };
		std::size_t _cols{ ColumnsExtent == std::dynamic_extent ? 0 : ColumnsExtent };

		constexpr void init_buf_2d_static(auto&& buf_2d, std::size_t rows, std::size_t cols)
		{
			_rows = rows;
			_cols = cols;

			auto idx = std::size_t{ 0 };
			for (const auto& row : buf_2d)
			{
				for (auto value : row)
				{
					_buf[idx++] = value;
				}
			}
		}

		void init_buf_2d_dynamic(auto&& buf_2d, std::size_t rows, std::size_t cols) // @TODO: ISSUE #20
		{
			_rows = rows;
			_cols = cols;
			_buf.reserve(rows * cols);

			for (const auto& row : buf_2d)
			{
				for (auto value : row)
				{
					_buf.push_back(value);
				}
			}
		}

	public:
		using buffer_type = Buffer;
		using value_type  = Value;

		matrix_base() = default; // @TODO: ISSUE #20

		[[nodiscard]] const buffer_type& buffer() const // @TODO: ISSUE #20
		{
			return _buf;
		}

		[[nodiscard]] decltype(auto) data() const // @TODO: ISSUE #20
		{
			return _buf.data();
		}

		[[nodiscard]] decltype(auto) begin() // @TODO: ISSUE #20, #21
		{
			return _buf.begin();
		}

		[[nodiscard]] decltype(auto) begin() const // @TODO: ISSUE #20, #21
		{
			return _buf.cbegin();
		}

		[[nodiscard]] decltype(auto) end() // @TODO: ISSUE #20, #21
		{
			return _buf.end();
		}

		[[nodiscard]] decltype(auto) end() const // @TODO: ISSUE #20, #21
		{
			return _buf.cend();
		}

		[[nodiscard]] decltype(auto) cbegin() // @TODO: ISSUE #20, #21
		{
			return _buf.cbegin();
		}

		[[nodiscard]] decltype(auto) cbegin() const // @TODO: ISSUE #20, #21
		{
			return _buf.cbegin();
		}

		[[nodiscard]] decltype(auto) cend() // @TODO: ISSUE #20, #21
		{
			return _buf.cend();
		}

		[[nodiscard]] decltype(auto) cend() const // @TODO: ISSUE #20, #21
		{
			return _buf.cend();
		}

		[[nodiscard]] decltype(auto) at(std::size_t row_idx, std::size_t col_idx) const // @TODO: ISSUE #20
		{
			if (row_idx >= _rows || col_idx >= _cols)
			{
				throw std::out_of_range("Access out of bounds!");
			}

			return operator()(row_idx, col_idx);
		}

		[[nodiscard]] value_type& operator()(std::size_t row_idx, std::size_t col_idx) // @TODO: ISSUE #20
		{
			auto index = idx_2d_to_1d(_cols, row_idx, col_idx);

			return _buf[index];
		}

		[[nodiscard]] const value_type& operator()(std::size_t row_idx,
			std::size_t col_idx) const // @TODO: ISSUE #20
		{
			auto index = idx_2d_to_1d(_cols, row_idx, col_idx);

			return _buf[index];
		}

		[[nodiscard]] auto rows() const // @TODO: ISSUE #20
		{
			return _rows;
		}

		[[nodiscard]] auto columns() const // @TODO: ISSUE #20
		{
			return _cols;
		}

		[[nodiscard]] auto rows_extent() const // @TODO: ISSUE #20
		{
			return RowsExtent;
		}

		[[nodiscard]] auto columns_extent() const // @TODO: ISSUE #20
		{
			return ColumnsExtent;
		}

		template<typename BaseBuffer, typename BaseValue, std::size_t BaseRowsExtent, std::size_t BaseColumnsExtent>
		friend inline void init_matrix_base_with_1d_rng(
			matrix_base<BaseBuffer, BaseValue, BaseRowsExtent, BaseColumnsExtent>& base,
			detail::range_arithmetic auto&& rng,
			std::size_t rows,
			std::size_t cols); // @TODO: ISSUE #20
	};

	template<typename BaseBuffer, typename BaseValue, std::size_t BaseRowsExtent, std::size_t BaseColumnsExtent>
	inline void init_matrix_base_with_1d_rng(
		matrix_base<BaseBuffer, BaseValue, BaseRowsExtent, BaseColumnsExtent>& base,
		detail::range_arithmetic auto&& rng,
		std::size_t rows,
		std::size_t cols) // @TODO: ISSUE #20
	{
		base._rows = rows;
		base._cols = cols;
		allocate_1d_buf_if_vector(base._buf, rows, cols);

		std::ranges::copy(rng, base._buf.begin());
	}
} // namespace matrixpp::detail
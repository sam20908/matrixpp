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

#include <cstddef>
#include <iterator>
#include <ranges>
#include <stdexcept>
#include <utility>

namespace matrixpp::detail
{
	template<typename Range>
	using range_2d_t = std::ranges::range_value_t<std::ranges::range_value_t<Range>>;

	/**
     * This is mainly for avoiding bug-prone code, because this calculation occurs
     * in a lot of places, and a typo can cause a lot of things to fail. It's
     * safer to wrap this calculation in a function, so the bug is easier to spot
     */
	[[nodiscard]] constexpr std::size_t idx_2d_to_1d(std::size_t cols, std::size_t row_idx, std::size_t col_idx)
	{
		return row_idx * cols + col_idx;
	}

	[[nodiscard]] constexpr auto dimension_not_zero_and_non_zero(std::size_t rows, std::size_t columns)
	{
		return (rows == 0 && columns != 0) || (rows != 0 && columns == 0);
	}

	[[nodiscard]] constexpr auto range_2d_dimensions(auto&& rng_2d)
	{
		auto begin = std::ranges::begin(rng_2d);
		auto rows  = std::ranges::size(rng_2d);
		auto cols  = rows > 0 ? std::ranges::size(*begin) : 0;

		if (rows > 1)
		{
			// We only need to check for equal row columns when the rows is 2+ because
			// 1 row means there's no other row to check and 0 rows is self explanatory

			for (auto&& row : rng_2d)
			{
				if (auto row_cols = std::ranges::size(row); row_cols != cols)
				{
					throw std::invalid_argument("2D initializer doesn't have equal row columns!");
				}
			}
		}

		return std::pair{ rows, cols };
	}

	inline void validate_matrices_same_size(const auto& left, const auto& right) // @TODO: ISSUE #20
	{
		if (left.rows() != right.rows() || left.columns() != right.columns())
		{
			throw std::runtime_error("Both matrices don't have the same size!");
		}
	}

	inline void validate_matrices_multipliable(const auto& left, const auto& right) // @TODO: ISSUE #20
	{
		if (left.columns() != right.rows())
		{
			throw std::runtime_error("Left matrix's columns is not equal to right matrix's rows!");
		}
	}

	inline void allocate_1d_buf_if_vector(const auto& buf, std::size_t rows, std::size_t cols) // @TODO: ISSUE #20
	{
		constexpr auto is_vec = requires
		{
			buf.push_back();
		};

		if constexpr (is_vec)
		{
			buf.resize(rows * cols);
		}
	}
} // namespace matrixpp::detail
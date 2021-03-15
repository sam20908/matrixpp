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

#include <cmath>
#include <compare>
#include <cstddef>
#include <limits>
#include <span>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>

namespace mpp::detail
{
	/**
	 * Helper type traits
	 */

	template<typename T>
	struct is_vector : std::false_type
	{
	};

	template<typename T>
	struct is_vector<std::vector<T>> : std::true_type
	{
	};

	/**
	 * Helper functions
	 */

	[[nodiscard]] constexpr auto idx_2d_to_1d(std::size_t columns, std::size_t row_idx, std::size_t col_idx) -> std::size_t
	{
		// This is mainly for avoiding bug-prone code, because this calculation occurs in a lot of places, and a typo
		// can cause a lot of things to fail. It's safer to wrap this calculation in a function, so the bug is easier to
		// spot. This also assumes that the storage of row-major

		return row_idx * columns + col_idx;
	}

	template<typename Range2D>
	[[nodiscard]] inline auto range_2d_dimensions(Range2D&& rng_2d)
		-> std::pair<std::size_t, std::size_t> // @TODO: ISSUE #20
	{
		const auto begin = std::ranges::begin(rng_2d);
		const auto rows  = std::ranges::size(rng_2d);
		const auto columns  = rows > 0 ? std::ranges::size(*begin) : std::size_t{};

		if (rows > 1)
		{
			// We only need to check for equal row columns when the rows is 2+ because 1 row means there's no other row
			// to check and 0 rows is self explanatory

			for (auto&& row : rng_2d)
			{
				if (const auto row_columns = std::ranges::size(std::forward<decltype(row)>(row)); row_columns != columns)
				{
					throw std::invalid_argument("Initializer doesn't have equal row columns!");
				}
			}
		}

		return { rows, columns };
	}

	[[nodiscard]] constexpr auto prefer_static_extent(std::size_t left_extent, std::size_t right_extent) -> std::size_t
	{
		if (left_extent != std::dynamic_extent || right_extent != std::dynamic_extent)
		{
			return left_extent != std::dynamic_extent ? left_extent : right_extent;
		}
		else
		{
			return std::dynamic_extent;
		}
	}

	inline void validate_same_size(const auto& left, const auto& right) // @TODO: ISSUE #20
	{
		if (left.rows() != right.rows() || left.columns() != right.columns())
		{
			throw std::invalid_argument("Both matrices must be the same size!");
		}
	}

	inline void validate_matrices_multipliable(const auto& left, const auto& right) // @TODO: ISSUE #20
	{
		if (left.columns() != right.rows())
		{
			throw std::invalid_argument("Left matrix's columns is not equal to right matrix's rows!");
		}
	}

	inline void validate_dimensions_for_identity(std::size_t rows, std::size_t columns) // @TODO: ISSUE #20
	{
		if (rows == 0 || columns == 0)
		{
			throw std::invalid_argument("Identity matrix cannot have a rank of 0!");
		}

		if (rows != columns)
		{
			throw std::invalid_argument("Identity matrix must be square!");
		}
	}

	template<typename InitializerValue>
	inline void
	allocate_1d_buf_if_vector(auto& buf, std::size_t rows, std::size_t columns, InitializerValue&& val) // @TODO: ISSUE #20
	{
		constexpr auto is_vec = is_vector<std::remove_cvref_t<decltype(buf)>>::value;

		if constexpr (is_vec)
		{
			buf.resize(rows * columns, std::forward<InitializerValue>(val));
		}
	}

	inline void reserve_1d_buf_if_vector(auto& buf, std::size_t rows, std::size_t columns) // @TODO: ISSUE #20
	{
		constexpr auto is_vec = is_vector<std::remove_cvref_t<decltype(buf)>>::value;

		if constexpr (is_vec)
		{
			buf.reserve(rows * columns);
		}
	}

	template<typename Value>
	inline void transform_1d_buf_into_identity(auto& buf, std::size_t n) // @TODO: ISSUE #20
	{
		// This assumes that the buffer is already filled with zeroes

		for (auto idx = std::size_t{}; idx < n; ++idx)
		{
			buf[idx_2d_to_1d(n, idx, idx)] = Value{ 1 };
		}
	}

	template<typename To, typename From>
	inline void mul_square_bufs(auto& buf, auto&& l_buf, auto&& r_buf, std::size_t n) // @TODO: ISSUE #20
	{
		for (auto row = std::size_t{}; row < n; ++row)
		{
			for (auto col = std::size_t{}; col < n; ++col)
			{
				auto result = To{};

				for (auto elem = std::size_t{}; elem < n; ++elem)
				{
					const auto left_idx  = idx_2d_to_1d(n, row, elem);
					const auto right_idx = idx_2d_to_1d(n, elem, col);

					result += static_cast<To>(l_buf[left_idx]) * static_cast<To>(r_buf[right_idx]);
				}

				const auto idx = idx_2d_to_1d(n, row, col);
				buf[idx]       = result;
			}
		}
	}

	template<typename To, bool FillLBuf>
	inline auto lu_decomp_common(std::size_t rows, std::size_t columns, auto& l_buf, auto& u_buf) -> To // @TODO: ISSUE #20
	{
		// Things this function expects from l_buf and u_buf:
		// 1. l_buf is already an identity buffer
		// 2. u_buf has the original values

		auto det = To{ 1 };

		// Compute the determinant while also compute LU Decomposition
		for (auto row = std::size_t{}; row < rows; ++row)
		{
			auto begin_idx     = idx_2d_to_1d(columns, row, std::size_t{});
			const auto end_idx = idx_2d_to_1d(columns, row, columns);

			for (auto col = std::size_t{}; col < row; ++col)
			{
				// This allows us to keep track of the row of the factor later on without having to manually
				// calculate from indexes
				auto factor_row_idx = idx_2d_to_1d(columns, col, col);

				const auto elem_idx = idx_2d_to_1d(columns, row, col);
				const auto factor   = u_buf[elem_idx] / u_buf[factor_row_idx] * -1;

				for (auto idx = begin_idx; idx < end_idx; ++idx)
				{
					u_buf[idx] += factor * u_buf[factor_row_idx++];
				}

				// Handle cases where we don't need to store the factors in a separate buffer (plain determinant
				// algorithm for example). This allows passing empty buffer as l_buf for optimization
				if constexpr (FillLBuf)
				{
					// L stores the opposite (opposite sign) of the factors used for U in the corresponding
					// location. But, to help optimize the calculation of inv(L), we can just leave the sign because
					// all the diagnoal elements below the diagonal element by 1 are the opposite sign, and it's
					// relatively easy to fix the values of other factors
					l_buf[elem_idx] = factor;
				}

				++begin_idx;
			}

			const auto diag_elem_idx = idx_2d_to_1d(columns, row, row);
			det *= u_buf[diag_elem_idx];
		}

		return det;
	}
} // namespace mpp::detail

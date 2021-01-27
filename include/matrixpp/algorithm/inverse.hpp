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

#include "../detail/algo_types.hpp"
#include "../detail/matrix_base.hpp"
#include "../detail/matrix_def.hpp"
#include "../detail/tag_invoke.hpp"
#include "../detail/utility.hpp"
#include "../utility/square.hpp"

#include <algorithm>
#include <ranges>
#include <type_traits>

namespace matrixpp
{
	namespace detail
	{
		template<typename To, typename Value, std::size_t RowsExtent, std::size_t ColumnsExtent>
		[[nodiscard]] inline auto inv_lu_decomp(const matrix<Value, RowsExtent, ColumnsExtent>& obj)
			-> matrix<To, RowsExtent, ColumnsExtent>
		{
			const auto rows = obj.rows();
			const auto cols = obj.columns();

			using inv_matrix_t = matrix<To, RowsExtent, ColumnsExtent>;
			auto inv_matrix    = inv_matrix_t{};

			// Handle special cases - avoid LU Decomposition
			if (rows == 0)
			{
				return inv_matrix;
			}

			auto inv_matrix_buf = typename inv_matrix_t::buffer_type{};
			allocate_1d_buf_if_vector(inv_matrix_buf, rows, cols);

			if (rows == 1)
			{
				const auto elem = static_cast<To>(obj(0, 0));

				if (accurate_equals(elem, To{ 0 }))
				{
					throw std::runtime_error("Inverse of a singular matrix doesn't exist!");
				}

				inv_matrix_buf[0] = To{ 1 } / elem;
			}

			auto det = lu_decomp_value_t{ 1 };

			if (rows == 2)
			{
				const auto element_1 = static_cast<lu_decomp_value_t>(obj(1, 1));
				const auto element_2 = static_cast<lu_decomp_value_t>(obj(0, 1));
				const auto element_3 = static_cast<lu_decomp_value_t>(obj(1, 0));
				const auto element_4 = static_cast<lu_decomp_value_t>(obj(0, 0));

				const auto ad = element_4 * element_1;
				const auto bc = element_2 * element_3;

				det = ad - bc;

				if (accurate_equals(det, lu_decomp_value_t{ 0 }))
				{
					throw std::runtime_error("Inverse of a singular matrix doesn't exist!");
				}

				const auto multiplier = lu_decomp_value_t{ 1 } / det;

				inv_matrix_buf[0] = static_cast<To>(multiplier * element_1);
				inv_matrix_buf[1] = static_cast<To>(multiplier * element_2 * -1);
				inv_matrix_buf[2] = static_cast<To>(multiplier * element_3 * -1);
				inv_matrix_buf[3] = static_cast<To>(multiplier * element_4);
			}

			if (rows >= 3)
			{
				using lu_decomp_matrix_t = matrix<lu_decomp_value_t, RowsExtent, ColumnsExtent>;
				using lu_decomp_buf_t    = typename lu_decomp_matrix_t::buffer_type;
				using lu_decomp_diff_t   = typename lu_decomp_matrix_t::difference_type;

				auto l_buf = lu_decomp_buf_t{};
				auto u_buf = lu_decomp_buf_t{};

				allocate_1d_buf_if_vector(u_buf, rows, cols);
				std::ranges::copy(obj, u_buf.begin());

				allocate_1d_buf_if_vector(l_buf, rows, cols);
				transform_1d_buf_into_identity<lu_decomp_value_t>(l_buf, rows);

				// Compute the determinant while also compute LU Decomposition
				for (auto row = std::size_t{ 0 }; row < rows; ++row)
				{
					const auto begin_idx = static_cast<lu_decomp_diff_t>(idx_2d_to_1d(cols, row, std::size_t{ 0 }));
					const auto end_idx   = static_cast<lu_decomp_diff_t>(idx_2d_to_1d(cols, row, cols));

					auto begin = std::next(u_buf.begin(), begin_idx);
					auto end   = std::next(u_buf.begin(), end_idx);

					for (auto col = std::size_t{ 0 }; col < row; ++col)
					{
						// This allows us to keep track of the row of the factor
						// later on without having to manually calculate from indexes
						auto factor_row_idx = idx_2d_to_1d(cols, col, col);

						const auto elem_idx = idx_2d_to_1d(cols, row, col);
						const auto factor   = u_buf[elem_idx] / u_buf[factor_row_idx] * -1;

						std::ranges::transform(begin, end, begin, [factor, factor_row_idx, &u_buf](auto elem) mutable {
							return factor * u_buf[factor_row_idx++] + elem;
						});

						// L stores the opposite (opposite sign) of the factors used for
						// U in the corresponding location. But, to help optimize the
						// calculation of inv(L), we can just leave the sign because
						// all the diagnoal elements below the diagonal element by 1
						// are the opposite sign, and it's relatively easy to fix the
						// values of other factors
						l_buf[elem_idx] = factor;

						++begin;
					}

					const auto pivot_idx = idx_2d_to_1d(cols, row, row);
					det *= u_buf[pivot_idx];
				}

				if (accurate_equals(det, lu_decomp_value_t{ 0 }))
				{
					throw std::runtime_error("Inverse of a singular matrix doesn't exist!");
				}

				// Compute inverse of L directly on the same buffer
				for (auto col = std::size_t{ 1 }; col < cols; ++col)
				{
					// Optimized version of forward-substitution which
					// skips making diagnoal 1's

					for (auto row = col + 1; row < rows; ++row)
					{
						const auto factor = l_buf[idx_2d_to_1d(cols, row, row - 1)] * -1;

						for (auto col_2 = std::size_t{ 0 }; col_2 < col; ++col_2)
						{
							const auto elem_above = l_buf[idx_2d_to_1d(cols, col, col_2)];
							const auto elem_idx   = idx_2d_to_1d(cols, row, col_2);

							l_buf[elem_idx] -= factor * elem_above;
						}
					}
				}

				// Compute inverse of U directly on the same buffer
				for (auto col = cols; col > 0; --col)
				{
					const auto col_idx    = col - 1;
					auto pivot_elem_idx   = idx_2d_to_1d(cols, col_idx, col_idx);
					const auto pivot_elem = u_buf[pivot_elem_idx];

					// Pivot can simply be replaced with the factor
					const auto pivot_factor = lu_decomp_value_t{ 1 } / pivot_elem;
					u_buf[pivot_elem_idx]   = pivot_factor;

					// Multiply every element to the right of the pivot by the
					// factor
					for (auto idx = cols - col; idx > 0; --idx)
					{
						u_buf[++pivot_elem_idx] *= pivot_factor;
					}

					for (auto row = col_idx; row > 0; --row)
					{
						// Use the pivot as the factor to compute the numbers
						// above the pivot in the same column (this works because)
						// the augmented matrix would have zeroes above the pivot
						const auto row_idx            = row - 1;
						const auto elem_idx           = idx_2d_to_1d(cols, row_idx, col_idx);
						const auto elem_before_factor = u_buf[elem_idx];
						u_buf[elem_idx]               = elem_before_factor * pivot_factor * -1;

						// Add the corresponding elements of the rows of the current
						// pivot onto the rows above
						for (auto col_2 = cols; col_2 > col; --col_2)
						{
							const auto col_2_idx      = col_2 - 1;
							auto pivot_row_idx        = idx_2d_to_1d(cols, col_idx, col_2_idx);
							const auto pivot_row_elem = u_buf[pivot_row_idx];

							const auto elem_idx = idx_2d_to_1d(cols, row_idx, col_2_idx);
							const auto elem     = u_buf[elem_idx];
							const auto factor   = elem_before_factor * -1;

							const auto new_elem = factor * pivot_row_elem + elem;
							u_buf[elem_idx]     = new_elem;
						}
					}
				}

				mul_square_bufs<To, lu_decomp_value_t>(inv_matrix_buf, std::move(u_buf), std::move(l_buf), rows);
			}

			init_matrix_base_with_1d_rng(inv_matrix, std::move(inv_matrix_buf), rows, cols);
			return inv_matrix;
		}

		template<typename To, typename Value, std::size_t RowsExtent, std::size_t ColumnsExtent>
		[[nodiscard]] inline auto inv_func(const matrix<Value, RowsExtent, ColumnsExtent>& obj)
			-> matrix<To, RowsExtent, ColumnsExtent> // @TODO: ISSUE #20
		{
			if (!square(obj))
			{
				throw std::runtime_error("Inverse of a non-square matrix doesn't exist!");
			}

			return inv_lu_decomp<To>(obj);
		}
	} // namespace detail

	struct inverse_t
	{
		template<typename Value, std::size_t RowsExtent, std::size_t ColumnsExtent>
		[[nodiscard]] friend constexpr auto tag_invoke(inverse_t, const matrix<Value, RowsExtent, ColumnsExtent>& obj)
			-> matrix<Value, RowsExtent, ColumnsExtent>
		{
			return detail::inv_func<Value>(obj);
		}

		template<typename To, typename Value, std::size_t RowsExtent, std::size_t ColumnsExtent>
		[[nodiscard]] friend constexpr auto tag_invoke(inverse_t,
			std::type_identity<To>,
			const matrix<Value, RowsExtent, ColumnsExtent>& obj) -> matrix<To, RowsExtent, ColumnsExtent>
		{
			return detail::inv_func<To>(obj);
		}

		template<typename... Args>
		[[nodiscard]] constexpr auto operator()(Args&&... args) const
			-> detail::tag_invoke_impl::tag_invoke_result_t<inverse_t, Args...>
		{
			return detail::tag_invoke_cpo(*this, std::forward<Args>(args)...);
		}
	};

	inline constexpr auto inverse = inverse_t{};
} // namespace matrixpp
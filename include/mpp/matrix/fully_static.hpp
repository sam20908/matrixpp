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

#include <mpp/detail/matrix/matrix_base.hpp>
#include <mpp/detail/matrix/matrix_def.hpp>
#include <mpp/detail/types/constraints.hpp>
#include <mpp/detail/utility/buffer_manipulators.hpp>
#include <mpp/detail/utility/utility.hpp>

#include <concepts>
#include <initializer_list>
#include <type_traits>
#include <utility>

namespace mpp
{
	template<detail::arithmetic Value, std::size_t RowsExtent, std::size_t ColumnsExtent, typename Allocator>
	class matrix :
		public detail::matrix_base<matrix<Value, RowsExtent, ColumnsExtent, Allocator>,
			typename configuration<override>::static_buffer<Value, RowsExtent, ColumnsExtent, Allocator>,
			Value,
			RowsExtent,
			ColumnsExtent,
			Allocator>
	{
		using base = detail::matrix_base<matrix<Value, RowsExtent, ColumnsExtent, Allocator>,
			typename configuration<override>::static_buffer<Value, RowsExtent, ColumnsExtent, Allocator>,
			Value,
			RowsExtent,
			ColumnsExtent,
			Allocator>;

		void fill_buffer_with_value(const Value& value) // @TODO: ISSUE #20
		{
			std::ranges::fill(base::buffer_, value);
		}

	public:
		using base::operator=;

		matrix() : base(RowsExtent, ColumnsExtent)
		{
			fill_buffer_with_value(Value{});
		}

		// clang-format off
		template<detail::matrix_with_value_convertible_to<Value> Matrix>
			requires (!std::same_as<std::remove_cvref_t<Matrix>, matrix<Value, RowsExtent, ColumnsExtent, Allocator>>)
		explicit matrix(Matrix&& matrix) : base(RowsExtent, ColumnsExtent) // @TODO: ISSUE #20
		{
			const auto mat_rows    = std::forward<Matrix>(matrix).rows();
			const auto mat_columns = std::forward<Matrix>(matrix).columns();

			base::assign_and_insert_from_1d_range(mat_rows, mat_columns, std::forward<Matrix>(matrix));
		}
		// clang-format on

		template<detail::range_1d_with_value_type_convertible_to<Value> Range>
		explicit matrix(std::size_t rows, std::size_t columns, Range&& range) :
			base(RowsExtent, ColumnsExtent) // @TODO: ISSUE #20
		{
			base::assign_and_insert_from_1d_range(rows, columns, std::forward<Range>(range));
		}

		template<std::convertible_to<Value> InitializerListValue>
		explicit matrix(std::initializer_list<std::initializer_list<InitializerListValue>> initializer_list_2d) :
			base(RowsExtent, ColumnsExtent) // @TODO: ISSUE #20
		{
			base::assign_and_insert_from_2d_range(initializer_list_2d);
		}

		template<detail::range_2d_with_value_type_convertible_to<Value> Range2D>
		explicit matrix(Range2D&& range_2d) : base(RowsExtent, ColumnsExtent) // @TODO: ISSUE #20
		{
			base::assign_and_insert_from_2d_range(std::forward<Range2D>(range_2d));
		}

		template<std::convertible_to<Value> Array2DValue>
		explicit matrix(const std::array<std::array<Array2DValue, ColumnsExtent>, RowsExtent>& array_2d) :
			base(RowsExtent, ColumnsExtent)
		{
			base::assign_and_insert_from_2d_range<false, false, false>(array_2d);
		}

		template<std::convertible_to<Value> Array2DValue>
		explicit matrix(std::array<std::array<Array2DValue, ColumnsExtent>, RowsExtent>&& array_2d) :
			base(RowsExtent, ColumnsExtent)
		{
			base::assign_and_insert_from_2d_range<false, false, false>(std::move(array_2d));
		}


		template<typename Expr, std::size_t ExprRowsExtent, std::size_t ExprColumnsExtent>
		explicit matrix(const detail::expr_base<Expr, Value, ExprRowsExtent, ExprColumnsExtent>& expr) :
			base(RowsExtent, ColumnsExtent) // @TODO: ISSUE #20
		{
			for (auto row = std::size_t{}, index = std::size_t{}; row < RowsExtent; ++row)
			{
				for (auto column = std::size_t{}; column < ColumnsExtent; ++column)
				{
					base::buffer_[index++] = expr(row, column);
				}
			}
		}

		explicit matrix(const Value& value) : base(RowsExtent, ColumnsExtent) // @TODO: ISSUE #20
		{
			fill_buffer_with_value(value);
		}

		explicit matrix(identity_tag, const Value& zero_value = Value{ 0 }, const Value& one_value = Value{ 1 }) :
			base(RowsExtent, ColumnsExtent) // @TODO: ISSUE #20
		{
			detail::make_identity_buffer(base::buffer_, RowsExtent, ColumnsExtent, zero_value, one_value);
		}

		// @FIXME: Allow callable's value return be convertible to value type
		template<detail::invocable_with_return_type<Value> Callable>
		explicit matrix(Callable&& callable) : base(RowsExtent, ColumnsExtent) // @TODO: ISSUE #20
		{
			std::ranges::generate(base::buffer_, std::forward<Callable>(callable));
		}

		template<std::convertible_to<Value> InitializerListValue>
		void assign(
			std::initializer_list<std::initializer_list<InitializerListValue>> initializer_list_2d) // @TODO: ISSUE #20
		{
			base::assign_and_insert_from_2d_range(initializer_list_2d);
		}

		template<detail::range_2d_with_value_type_convertible_to<Value> Range2D>
		void assign(Range2D&& range_2d) // @TODO: ISSUE #20
		{
			base::assign_and_insert_from_2d_range(std::forward<Range2D>(range_2d));
		}

		template<std::convertible_to<Value> Array2DValue>
		void assign(const std::array<std::array<Array2DValue, ColumnsExtent>, RowsExtent>& array_2d)
		{
			base::assign_and_insert_from_2d_range(array_2d);
		}

		template<std::convertible_to<Value> Array2DValue>
		void assign(std::array<std::array<Array2DValue, ColumnsExtent>, RowsExtent>&& array_2d)
		{
			base::assign_and_insert_from_2d_range(std::move(array_2d));
		}

		// clang-format off
		template<detail::matrix_with_value_convertible_to<Value> Matrix>
			requires (!std::same_as<std::remove_cvref_t<Matrix>, matrix<Value, RowsExtent, ColumnsExtent, Allocator>>)
		void assign(Matrix&& matrix) // @TODO: ISSUE #20
		{
			const auto mat_rows    = std::forward<Matrix>(matrix).rows();
			const auto mat_columns = std::forward<Matrix>(matrix).columns();

			base::assign_and_insert_from_1d_range(mat_rows, mat_columns, std::forward<Matrix>(matrix));
		}
		// clang-format on
	};
} // namespace mpp

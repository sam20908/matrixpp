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
#include <mpp/detail/utility/utility.hpp>

#include <initializer_list>
#include <stdexcept>
#include <type_traits>
#include <utility>

namespace mpp
{
	template<detail::arithmetic Value, typename Allocator>
	class matrix<Value, dynamic, dynamic, Allocator> :
		public detail::matrix_dynamic_base<matrix<Value, dynamic, dynamic, Allocator>,
			typename configuration<override>::dynamic_buffer<Value, dynamic, dynamic, Allocator>,
			Value,
			dynamic,
			dynamic,
			Allocator>
	{
		using base = detail::matrix_dynamic_base<matrix<Value, dynamic, dynamic, Allocator>,
			typename configuration<override>::dynamic_buffer<Value, dynamic, dynamic, Allocator>,
			Value,
			dynamic,
			dynamic,
			Allocator>;

	public:
		using base::operator=;

		matrix() noexcept(std::is_default_constructible_v<Allocator>) : base(0, 0, Allocator{}) {} // @TODO: ISSUE #20

		explicit matrix(const Allocator& allocator) noexcept : base(0, 0, allocator) {} // @TODO: ISSUE #20

		matrix(const matrix& right, const Allocator& allocator) :
			base(right.rows_, right.columns_, right.buffer_, allocator) // @TODO: ISSUE #20
		{
		}

		matrix(matrix&& right, const Allocator& allocator) :
			base(std::move(right.rows_),
				std::move(right.columns_),
				std::move(right.buffer_),
				allocator) // @TODO: ISSUE #20
		{
		}

		// clang-format off
		template<detail::matrix_with_value_convertible_to<Value> Matrix>
			requires (!std::same_as<std::remove_cvref_t<Matrix>, matrix<Value, dynamic, dynamic, Allocator>>)
		explicit matrix(Matrix&& matrix, const Allocator& allocator = Allocator{}) :
			base(0, 0, allocator) // @TODO: ISSUE #20
		{
			const auto mat_rows    = std::forward<Matrix>(matrix).rows();
			const auto mat_columns = std::forward<Matrix>(matrix).columns();

			base::assign_and_insert_from_1d_range(mat_rows,
				mat_columns,
				std::forward<Matrix>(matrix));
		}
		// clang-format on

		template<detail::range_1d_with_value_type_convertible_to<Value> Range>
		explicit matrix(std::size_t rows,
			std::size_t columns,
			Range&& range,
			const Allocator& allocator = Allocator{}) :
			base(0, 0, allocator) // @TODO: ISSUE #20
		{
			base::assign_and_insert_from_1d_range(rows, columns, std::forward<Range>(range));
		}

		template<std::convertible_to<Value> InitializerListValue>
		explicit matrix(std::initializer_list<std::initializer_list<InitializerListValue>> initializer_list_2d,
			const Allocator allocator = Allocator{}) :
			base(0, 0, allocator) // @TODO: ISSUE #20
		{
			base::assign_and_insert_from_2d_range(initializer_list_2d);
		}

		template<detail::range_2d_with_value_type_convertible_to<Value> Range2D>
		explicit matrix(Range2D&& range_2d, const Allocator allocator = Allocator{}) :
			base(0, 0, allocator) // @TODO: ISSUE #20
		{
			base::assign_and_insert_from_2d_range(std::forward<Range2D>(range_2d));
		}

		template<typename Expr, std::size_t ExprRowsExtent, std::size_t ExprColumnsExtent>
		explicit matrix(const detail::expr_base<Expr, Value, ExprRowsExtent, ExprColumnsExtent>& expr,
			const Allocator allocator = Allocator{}) :
			base(0, 0, allocator) // @TODO: ISSUE #20
		{
			base::initialize_from_expression_unchecked(expr.rows(), expr.columns(), expr);
		}

		matrix(std::size_t rows, std::size_t columns, const Allocator& allocator = Allocator{}) :
			base(rows, columns, rows * columns, Value{}, allocator) // @TODO: ISSUE #20
		{
		}

		matrix(std::size_t rows, std::size_t columns, const Value& value, const Allocator& allocator = Allocator{}) :
			base(rows, columns, rows * columns, value, allocator) // @TODO: ISSUE #20
		{
		}

		matrix(std::size_t rows,
			std::size_t columns,
			identity_tag,
			const Value& zero_value    = Value{ 0 },
			const Value& one_value     = Value{ 1 },
			const Allocator& allocator = Allocator{}) :
			base(rows, columns, allocator) // @TODO: ISSUE #20
		{
			detail::make_identity_buffer(base::buffer_, rows, columns, zero_value, one_value);
		}

		// @FIXME: Allow callable's value return be convertible to value type
		template<detail::invocable_with_return_type<Value> Callable>
		matrix(std::size_t rows, std::size_t columns, Callable&& callable, const Allocator& allocator = Allocator{}) :
			base(rows, columns, allocator) // @TODO: ISSUE #20
		{
			base::initialize_buffer_from_callable_unchecked(rows, columns, std::forward<Callable>(callable));
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

		// clang-format off
		template<detail::matrix_with_value_convertible_to<Value> Matrix>
			requires (!std::same_as<std::remove_cvref_t<Matrix>, matrix<Value, dynamic, dynamic, Allocator>>)
		void assign(Matrix&& matrix) // @TODO: ISSUE #20
		{
			const auto mat_rows    = std::forward<Matrix>(matrix).rows();
			const auto mat_columns = std::forward<Matrix>(matrix).columns();

			base::assign_and_insert_from_1d_range(mat_rows,
				mat_columns,
				std::forward<Matrix>(matrix));
		}
		// clang-format on
	};
} // namespace mpp

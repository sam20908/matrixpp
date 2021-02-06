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

#include <mpp/detail/expr_binary_constant_op.hpp>
#include <mpp/detail/expr_binary_op.hpp>
#include <mpp/detail/utility.hpp>
#include <mpp/matrix.hpp>

#include <algorithm>
#include <cstddef>
#include <functional>
#include <type_traits>

namespace mpp
{
	namespace detail
	{
		using mul_constant_op_t = decltype(
			[](auto&& left, auto&& right, std::size_t row_idx, std::size_t col_idx) -> decltype(left(row_idx, col_idx) *
																								right) {
				return left(row_idx, col_idx) * right;
			});
		using mul_op_t = decltype([](auto&& left, auto&& right, std::size_t row_idx, std::size_t col_idx) ->
			typename std::decay_t<decltype(left)>::value_type {
				using value_type = typename std::decay_t<decltype(left)>::value_type;

				const auto left_cols = left.columns();
				auto result          = value_type{ 0 };

				for (auto index = std::size_t{ 0 }; index < left_cols; ++index)
				{
					result += left(row_idx, index) * right(index, col_idx);
				}

				return result;
			});
	} // namespace detail

	template<typename Base, typename Value, std::size_t RowsExtent, std::size_t ColumnsExtent>
	[[nodiscard]] inline auto operator*(const detail::expr_base<Base, Value, RowsExtent, ColumnsExtent>& obj,
		Value constant) -> detail::expr_binary_constant_op<RowsExtent,
		ColumnsExtent,
		detail::expr_base<Base, Value, RowsExtent, ColumnsExtent>,
		Value,
		detail::mul_constant_op_t> // @TODO: ISSUE #20
	{
		return { obj, constant, obj.rows(), obj.columns() };
	}

	template<typename Base, typename Value, std::size_t RowsExtent, std::size_t ColumnsExtent>
	[[nodiscard]] inline auto operator*(Value constant,
		const detail::expr_base<Base, Value, RowsExtent, ColumnsExtent>& obj)
		-> detail::expr_binary_constant_op<RowsExtent,
			ColumnsExtent,
			detail::expr_base<Base, Value, RowsExtent, ColumnsExtent>,
			Value,
			detail::mul_constant_op_t> // @TODO: ISSUE #20
	{
		return { obj, constant, obj.rows(), obj.columns() };
	}

	template<typename LeftBase,
		typename RightBase,
		typename Value,
		std::size_t LeftRowsExtent,
		std::size_t LeftColumnsExtent,
		std::size_t RightRowsExtent,
		std::size_t RightColumnsExtent>
	[[nodiscard]] inline auto operator*(
		const detail::expr_base<LeftBase, Value, LeftRowsExtent, LeftColumnsExtent>& left,
		const detail::expr_base<RightBase, Value, RightRowsExtent, RightColumnsExtent>& right)
		-> detail::expr_binary_op<LeftRowsExtent,
			RightColumnsExtent,
			detail::expr_base<LeftBase, Value, LeftRowsExtent, LeftColumnsExtent>,
			detail::expr_base<RightBase, Value, RightRowsExtent, RightColumnsExtent>,
			detail::mul_op_t> // @TODO: ISSUE #20
	{
		detail::validate_matrices_multipliable(left, right);

		return { left, right, left.rows(), right.columns() };
	}

	template<typename Value, std::size_t RowsExtent, std::size_t ColumnsExtent>
	inline auto operator*=(matrix<Value, RowsExtent, ColumnsExtent>& obj, Value constant)
		-> matrix<Value, RowsExtent, ColumnsExtent>& // @TODO: ISSUE #20
	{
		std::ranges::transform(obj, obj.begin(), std::bind_front(std::multiplies<>{}, constant));

		return obj;
	}
} // namespace mpp
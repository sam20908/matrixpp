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

#include "expr_base.hpp"

#include <cstddef>
#include <stdexcept>
#include <utility>

namespace matrixpp::detail
{
	/**
     * Binary expression object (no operand is a constant)
     */
	template<std::size_t RowsExtent, std::size_t ColumnsExtent, typename Left, typename Right, typename Op>
	class [[nodiscard]] expr_binary_op :
		public expr_base<expr_binary_op<RowsExtent, ColumnsExtent, Left, Right, Op>,
			typename Left::value_type,
			RowsExtent,
			ColumnsExtent>
	{
		// Store both operands by reference to avoid copying them
		const Left& _left;
		const Right& _right;

		const Op& _op;

		// "Knowing" the size of the resulting matrix allows performing validation on expression objects
		std::size_t _result_rows;
		std::size_t _result_cols;

	public:
		using value_type = typename Left::value_type;

		expr_binary_op(const Left& left,
			const Right& right,
			const Op& op,
			std::size_t result_rows,
			std::size_t result_cols) // @TODO: ISSUE #20
			:
			_left(left),
			_right(right),
			_op(op),
			_result_rows(result_rows),
			_result_cols(result_cols)
		{
		}

		[[nodiscard]] auto rows() const // @TODO: ISSUE #20
		{
			return _result_rows;
		}

		[[nodiscard]] auto columns() const // @TODO: ISSUE #20
		{
			return _result_cols;
		}

		[[nodiscard]] decltype(auto) at(std::size_t row_idx, std::size_t col_idx) const // @TODO: ISSUE #20
		{
			if (row_idx >= _result_rows || col_idx >= _result_cols)
			{
				throw std::out_of_range("Access out of range!");
			}

			return operator()(row_idx, col_idx);
		}

		[[nodiscard]] decltype(auto) operator()(std::size_t row_idx, std::size_t col_idx) const // @TODO: ISSUE #20
		{
			return _op(_left, _right, row_idx, col_idx);
		}
	};
} // namespace matrixpp::detail
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

#include "../detail/tag_invoke.hpp"
#include "../matrix.hpp"

#include <cstddef>

namespace matrixpp
{
	struct square_t
	{
		template<typename Value, std::size_t RowsExtent, std::size_t ColumnsExtent>
		[[nodiscard]] friend constexpr auto tag_invoke(square_t, const matrix<Value, RowsExtent, ColumnsExtent>& obj)
			-> bool
		{
			return obj.rows() == obj.columns();
		}

		template<typename... Args>
		[[nodiscard]] constexpr auto operator()(Args&&... args) const
			-> detail::tag_invoke_impl::tag_invoke_result_t<square_t, Args...>
		{
			return tag_invoke(*this, std::forward<Args>(args)...);
		}
	};

	inline constexpr auto square = square_t{};
} // namespace matrixpp
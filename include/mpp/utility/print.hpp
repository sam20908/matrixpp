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

#include <mpp/detail/utility/cpo_base.hpp>
#include <mpp/detail/utility/print_helpers.hpp>
#include <mpp/matrix.hpp>

#include <cstddef>
#include <iosfwd>
#include <sstream>

namespace mpp
{
	struct print_matrix_t : public detail::cpo_base<print_matrix_t>
	{
		template<typename Value, std::size_t RowsExtent, std::size_t ColumnsExtent, typename Allocator>
		friend inline auto tag_invoke(print_matrix_t, const matrix<Value, RowsExtent, ColumnsExtent, Allocator>& obj)
			-> void
		{
			auto message_stream = std::stringstream{};
			detail::insert_expr_content_into_out_stream(message_stream, obj, "");

			std::cout << message_stream.str();
		}
	};

	template<typename Value, std::size_t RowsExtent, std::size_t ColumnsExtent, typename Alloc>
	auto operator<<(std::ostream& os, const matrix<Value, RowsExtent, ColumnsExtent, Alloc>& obj) -> std::ostream&
	{
		detail::insert_expr_content_into_out_stream(os, obj, "");
		return os;
	}

	inline constexpr auto print_matrix = print_matrix_t{};
} // namespace mpp

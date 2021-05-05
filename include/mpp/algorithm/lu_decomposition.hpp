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

#include <mpp/detail/utility/algorithm_helpers.hpp>
#include <mpp/detail/utility/cpo_base.hpp>
#include <mpp/detail/utility/exception_messages.hpp>
#include <mpp/utility/square.hpp>
#include <mpp/matrix.hpp>

#include <cstddef>
#include <memory>
#include <utility>

namespace mpp
{
	namespace detail
	{
		template<typename To,
			bool CheckSquare,
			typename ToAllocator,
			typename Value,
			std::size_t RowsExtent,
			std::size_t ColumnsExtent,
			typename Allocator>
		auto lu_decomposition_func(const matrix<Value, RowsExtent, ColumnsExtent, Allocator>& obj)
			-> std::pair<matrix<To, RowsExtent, ColumnsExtent, ToAllocator>,
				matrix<To, RowsExtent, ColumnsExtent, ToAllocator>> // @TODO: ISSUE #20
		{
			if constexpr (CheckSquare)
			{
				if (!square(obj))
				{
					throw std::logic_error(MATRIX_NOT_SQUARE);
				}
			}

			using calc_matrix_t   = matrix<default_floating_type,
                RowsExtent,
                ColumnsExtent,
                typename std::allocator_traits<Allocator>::template rebind_alloc<default_floating_type>>;
			using result_matrix_t = matrix<To, RowsExtent, ColumnsExtent, ToAllocator>;

			const auto rows    = obj.rows();
			const auto columns = obj.columns();

			auto l_buffer = typename calc_matrix_t::buffer_type{};
			auto u_buffer = typename calc_matrix_t::buffer_type{};

			// @TODO: Should do a direct copy initialization instead
			allocate_buffer_if_vector(u_buffer, rows, columns, default_floating_type{});
			std::ranges::copy(obj, u_buffer.begin());

			make_identity_buffer<false>(l_buffer, rows, columns, default_floating_type{}, default_floating_type{ 1 });

			lu_generic<default_floating_type, true, false>(rows, columns, l_buffer, u_buffer);

			return { result_matrix_t{ rows, columns, std::move(l_buffer) },
				result_matrix_t{ rows, columns, std::move(u_buffer) } };
		}
	} // namespace detail

	struct lu_decomposition_t : public detail::cpo_base<lu_decomposition_t>
	{
		template<typename Value,
			std::size_t RowsExtent,
			std::size_t ColumnsExtent,
			typename Allocator,
			typename ToAllocator = Allocator>
		friend inline auto tag_invoke(lu_decomposition_t,
			const matrix<Value, RowsExtent, ColumnsExtent, Allocator>& obj,
			std::type_identity<ToAllocator> = {}) -> std::pair<matrix<Value, RowsExtent, ColumnsExtent, ToAllocator>,
			matrix<Value, RowsExtent, ColumnsExtent, ToAllocator>> // @TODO: ISSUE #20
		{
			return detail::lu_decomposition_func<Value, detail::configuration_use_safe, ToAllocator>(obj);
		}

		template<typename Value,
			std::size_t RowsExtent,
			std::size_t ColumnsExtent,
			typename Allocator,
			typename ToAllocator = Allocator>
		friend inline auto tag_invoke(lu_decomposition_t,
			const matrix<Value, RowsExtent, ColumnsExtent, Allocator>& obj,
			unsafe_tag,
			std::type_identity<ToAllocator> = {}) -> std::pair<matrix<Value, RowsExtent, ColumnsExtent, ToAllocator>,
			matrix<Value, RowsExtent, ColumnsExtent, ToAllocator>> // @TODO: ISSUE #20

		{
			return detail::lu_decomposition_func<Value, false, ToAllocator>(obj);
		}

		template<typename To,
			typename Value,
			std::size_t RowsExtent,
			std::size_t ColumnsExtent,
			typename Allocator,
			typename ToAllocator = typename std::allocator_traits<Allocator>::template rebind_alloc<To>>
		friend inline auto tag_invoke(lu_decomposition_t,
			std::type_identity<To>,
			const matrix<Value, RowsExtent, ColumnsExtent, Allocator>& obj,
			std::type_identity<ToAllocator> = {}) -> std::pair<matrix<To, RowsExtent, ColumnsExtent, ToAllocator>,
			matrix<To, RowsExtent, ColumnsExtent, ToAllocator>> // @TODO: ISSUE #20

		{
			return detail::lu_decomposition_func<To, detail::configuration_use_safe, ToAllocator>(obj);
		}


		template<typename To,
			typename Value,
			std::size_t RowsExtent,
			std::size_t ColumnsExtent,
			typename Allocator,
			typename ToAllocator = typename std::allocator_traits<Allocator>::template rebind_alloc<To>>
		friend inline auto tag_invoke(lu_decomposition_t,
			std::type_identity<To>,
			const matrix<Value, RowsExtent, ColumnsExtent, Allocator>& obj,
			unsafe_tag,
			std::type_identity<ToAllocator> = {}) -> std::pair<matrix<To, RowsExtent, ColumnsExtent, ToAllocator>,
			matrix<To, RowsExtent, ColumnsExtent, ToAllocator>> // @TODO: ISSUE #20

		{
			return detail::lu_decomposition_func<To, false, ToAllocator>(obj);
		}
	};

	inline constexpr auto lu_decomposition = lu_decomposition_t{};
} // namespace mpp

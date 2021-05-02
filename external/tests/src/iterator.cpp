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

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4459)
#endif

#include <boost/ut.hpp>

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#include <mpp/matrix.hpp>

using namespace boost::ut::literals;
using namespace boost::ut::bdd;
using namespace boost::ut;

template<std::size_t RowsExtent, std::size_t ColumnsExtent>
void test_iter_semantics(const auto& rng, auto... dimension_args)
{
	const auto mat = mpp::matrix<int, RowsExtent, ColumnsExtent>{ rng, dimension_args... };

	auto begin  = mat.begin();
	auto cbegin = mat.cbegin();
	auto end    = mat.end() - 1;
	auto cend   = mat.cend() - 1;

	expect(*begin == 1);
	expect(*cbegin == 1);
	expect(*end == 6);
	expect(*cend == 6);

	auto begin_old  = begin++;
	auto cbegin_old = cbegin++;

	expect(*begin == 2);
	expect(*cbegin == 2);
	expect(*begin_old == 1);
	expect(*cbegin_old == 1);

	++begin_old;
	++cbegin_old;

	expect(begin == begin_old);
	expect(cbegin == cbegin_old);

	begin += 2;
	cbegin += 2;
	end -= 2;
	cend -= 2;

	expect(begin == end);
	expect(cbegin == cend);

	auto rbegin  = mat.rbegin();
	auto crbegin = mat.crbegin();
	auto rend    = mat.rend() - 1;
	auto crend   = mat.crend() - 1;

	expect(*rbegin == 6);
	expect(*crbegin == 6);
	expect(*rend == 1);
	expect(*crend == 1);

	auto begin_2 = mat.begin();
	begin_2 += { 1, 2 };
	expect(*begin_2 == 6);

	begin_2 -= { 1, 0 };
	expect(*begin_2 == 3);

	auto begin_3 = begin_2 + std::pair{ 0, 0 };
	expect(begin_3 == begin_2);
}

template<std::size_t RowsExtent, std::size_t ColumnsExtent>
void test_iter_semantics_write(const auto& rng)
{
	auto mat = mpp::matrix<int, RowsExtent, ColumnsExtent>{ rng };

	auto cbegin  = mat.cbegin();
	auto cend    = mat.cend() - 1;
	auto crbegin = mat.crbegin();
	auto crend   = mat.crend() - 1;

	expect(*cbegin == 1_i);
	expect(*cend == 6_i);
	expect(*crbegin == 6_i);
	expect(*crend == 1_i);

	auto begin  = mat.begin();
	auto end    = mat.end() - 1;
	auto rbegin = mat.rbegin();
	auto rend   = mat.rend() - 1;

	*begin = 2;
	*end   = 2;
	expect(*begin == 2);
	expect(*end == 2);

	*rbegin = 3;
	*rend   = 4;
	expect(*end == 3);
	expect(*begin == 4);
}

int main()
{
	feature("Standard conforming iterators") = []() {
		using static_mat      = mpp::matrix<int, 1, 1>;
		using dyn_mat         = mpp::matrix<int>;
		using dyn_rows_mat    = mpp::matrix<int, mpp::dynamic, 1>;
		using dyn_columns_mat = mpp::matrix<int, 1, mpp::dynamic>;

		using static_iterator            = typename static_mat::iterator;
		using static_const_iterator      = typename static_mat::const_iterator;
		using dyn_iterator               = typename dyn_mat::iterator;
		using dyn_const_iterator         = typename dyn_mat::const_iterator;
		using dyn_rows_iterator          = typename dyn_rows_mat::iterator;
		using dyn_rows_const_iterator    = typename dyn_rows_mat::const_iterator;
		using dyn_columns_iterator       = typename dyn_columns_mat::iterator;
		using dyn_columns_const_iterator = typename dyn_columns_mat::const_iterator;

		given("Mutable and immutable iterators should meet contiguous_iterator") = [&]() {
			expect(constant<std::contiguous_iterator<static_iterator>>);
			expect(constant<std::contiguous_iterator<static_const_iterator>>);
			expect(constant<std::contiguous_iterator<dyn_iterator>>);
			expect(constant<std::contiguous_iterator<dyn_const_iterator>>);
			expect(constant<std::contiguous_iterator<dyn_rows_iterator>>);
			expect(constant<std::contiguous_iterator<dyn_rows_const_iterator>>);
			expect(constant<std::contiguous_iterator<dyn_columns_iterator>>);
			expect(constant<std::contiguous_iterator<dyn_columns_const_iterator>>);
		};

		given("Mutable iterators should meet output_iterator") = [&]() {
			expect(constant<std::output_iterator<static_iterator, int>>);
			expect(constant<std::output_iterator<dyn_iterator, int>>);
		};
	};

	feature("Iterator semantics") = []() {
		// @NOTE: Every test is referencing the below rng for convenience
		const auto rng = std::vector<std::vector<int>>{ { 1, 2, 3 }, { 4, 5, 6 } };

		given("Mutable and immutable iterators should have right semantics") = [&]() {
			test_iter_semantics<2, 3>(rng);
			test_iter_semantics<mpp::dynamic, mpp::dynamic>(rng);
			test_iter_semantics<mpp::dynamic, 3>(rng);
			test_iter_semantics<2, mpp::dynamic>(rng);
		};

		given("Mutable iterators should have right mutable semantics") = [&]() {
			test_iter_semantics_write<2, 3>(rng);
			test_iter_semantics_write<mpp::dynamic, mpp::dynamic>(rng);
			test_iter_semantics_write<mpp::dynamic, 3>(rng);
			test_iter_semantics_write<2, mpp::dynamic>(rng);
		};
	};

	return 0;
}

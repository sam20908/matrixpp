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

// RUN: cd %binary_dir
// RUN: cmake -DTEST_NAME=deduction_guides -DTEST_SOURCE=%s -B build/deduction_guides
// RUN: cmake --build build/deduction_guides --target deduction_guides

#include <mpp/matrix.hpp>

#include "../../include/dummy_expr.hpp"

#include <vector>

int main()
{
	// 2D initializer list
	(void)mpp::matrix{ { 7, 3, 1 }, { 8, 8, 2 }, { 5, 8, 2 } };

	// Expression object
	const auto matrix = mpp::matrix<int>{};
	(void)mpp::matrix{ mpp_test::detail::dummy_expr{ matrix } };

	// 2D range
	const auto rng_2d = std::vector{ std::vector{ 1, 2, 3 }, std::vector{ 1, 2, 3 } };
	(void)mpp::matrix{ rng_2d };

	return 0;
}
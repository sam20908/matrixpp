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

#include <benchmark/benchmark.h>
#include <matrixpp/matrix/fully_dynamic.hpp>

// Benchmarking any other matrices (at least one side has compile time specified extent)
// isn't viable because no functionality is provided to access the range at compile time

static void Initialization_Fill(benchmark::State& state)
{
	for (auto _ : state)
	{
		(void)matrixpp::matrix<int>{ state.range(), state.range(), 0 };
	}

	state.counters["Rows"]    = state.range();
	state.counters["Columns"] = state.range();
}

BENCHMARK(Initialization_Fill)->RangeMultiplier(2)->Range(8, 8 << 10);
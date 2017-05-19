// Copyright 2017 Kyle Mayes
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <accelerando.hpp>

ACCEL_BENCHMARKS

#include <algorithm>
#include <list>
#include <map>
#include <numeric>
#include <unordered_map>

//================================================
// Basic
//================================================

std::vector<uint64_t> generate_integers(uint64_t size) {
    std::vector<uint64_t> integers;
    integers.reserve(size);
    std::generate_n(std::back_inserter(integers), size, [&]() { return 1 + integers.size(); });
    return integers;
}

const static std::vector<uint64_t> INTEGERS = generate_integers(1024);

BENCHMARK(Accumulate) {
    accel::retain(std::accumulate(INTEGERS.begin(), INTEGERS.end(), 0));
}

BENCHMARK(Loop) {
    uint64_t sum = 0;
    for (auto integer : INTEGERS) {
        sum += integer;
    }
    accel::retain(sum);
}

//================================================
// Parameterized
//================================================

BENCHMARK_P(Fibonacci, uint64_t nth) {
    uint64_t a = 0, b = 1;
    for (uint64_t index = 0; index < nth; ++index) {
        auto temp = a + b;
        a = b;
        b = temp;
        accel::retain(a);
        accel::retain(b);
    }
}

BENCHMARK_P_INSTANCE(Fibonacci, 16, 16)
BENCHMARK_P_INSTANCE(Fibonacci, 32, 32)
BENCHMARK_P_INSTANCE(Fibonacci, 64, 64)

//================================================
// Templated
//================================================

BENCHMARK_T(Emplace, template <class...> class C) {
    C<uint64_t, uint64_t> container;
    for (auto integer : INTEGERS) {
        container.emplace(integer, integer);
    }
    accel::retain(container);
}

BENCHMARK_T_INSTANCE(Emplace, Map, std::map)
BENCHMARK_T_INSTANCE(Emplace, UnorderedMap, std::unordered_map)

//================================================
// Paramaterized and Templated
//================================================

BENCHMARK_PT(Find, ACCEL_GROUP(template <class...> class C), uint64_t nth) {
    const static C<uint64_t> CONTAINER(INTEGERS.begin(), INTEGERS.end());
    accel::retain(std::find(CONTAINER.begin(), CONTAINER.end(), nth));
}

BENCHMARK_PT_INSTANCE(Find, List10, ACCEL_GROUP(std::list), 10)
BENCHMARK_PT_INSTANCE(Find, List100, ACCEL_GROUP(std::list), 100)
BENCHMARK_PT_INSTANCE(Find, List1000, ACCEL_GROUP(std::list), 1000)

BENCHMARK_PT_INSTANCE(Find, Vector10, ACCEL_GROUP(std::vector), 10)
BENCHMARK_PT_INSTANCE(Find, Vector100, ACCEL_GROUP(std::vector), 100)
BENCHMARK_PT_INSTANCE(Find, Vector1000, ACCEL_GROUP(std::vector), 1000)

//================================================
// Fixtures
//================================================

// Any subset of the member functions below may be provided to implement a fixture.
struct Integers : public accel::Benchmark {
    // Static fields are shared between benchmarks using the same fixture.
    static std::vector<uint64_t> static_integers;

    // This member function is called before any benchmark that uses this fixture is run.
    static void static_set_up() { static_integers = {1, 2, 3}; }
    // This member function is called after all benchmarks that use this fixture have been run.
    static void static_tear_down() { static_integers.clear(); }

    // Non-static fields are not shared between benchmarks using the same fixture.
    std::vector<uint64_t> integers;

    // This member function is called before each benchmark that uses this fixture is run.
    virtual void set_up() override { integers = {4, 5, 6}; }
    // This member function is called after each benchmark that uses this fixture is run.
    virtual void tear_down() override { integers.clear(); }
};

std::vector<uint64_t> Integers::static_integers;

BENCHMARK_F(Integers, Benchmark) {
    accel::retain(static_integers);
    accel::retain(integers);
}

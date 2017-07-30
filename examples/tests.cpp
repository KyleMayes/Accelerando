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

ACCEL_TESTS

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

TEST(Accumulate) {
    ASSERT_EQ(std::accumulate(INTEGERS.begin(), INTEGERS.end(), 0), 524800u);
}

TEST(Loop) {
    uint64_t sum = 0;
    for (auto integer : INTEGERS) {
        sum += integer;
    }
    ASSERT_EQ(sum, 524800u);
}

//================================================
// Parameterized
//================================================

TEST_P(Parity, uint64_t integer) {
    ASSERT_EQ(integer % 2, 0);
    ASSERT_NE(integer % 2, 1);
}

TEST_P_INSTANCE(Parity, 2, 2)
TEST_P_INSTANCE(Parity, 4, 4)
TEST_P_INSTANCE(Parity, 8, 8)

//================================================
// Templated
//================================================

TEST_T(Emplace, template <class...> class C) {
    C<uint64_t, uint64_t> container;
    for (auto integer : INTEGERS) {
        container.emplace(integer, integer);
    }
    for (auto integer : INTEGERS) {
        auto iterator = container.find(integer);
        ASSERT_NE(iterator, container.end());
        ASSERT_EQ(iterator->second, integer);
    }
}

TEST_T_INSTANCE(Emplace, Map, std::map)
TEST_T_INSTANCE(Emplace, UnorderedMap, std::unordered_map)

//================================================
// Paramaterized and Templated
//================================================

TEST_PT(Find, ACCEL_GROUP(template <class...> class C), uint64_t nth) {
    const static C<uint64_t> CONTAINER(INTEGERS.begin(), INTEGERS.end());
    ASSERT_NE(std::find(CONTAINER.begin(), CONTAINER.end(), nth), CONTAINER.end());
}

TEST_PT_INSTANCE(Find, List10, ACCEL_GROUP(std::list), 10)
TEST_PT_INSTANCE(Find, List100, ACCEL_GROUP(std::list), 100)
TEST_PT_INSTANCE(Find, List1000, ACCEL_GROUP(std::list), 1000)

TEST_PT_INSTANCE(Find, Vector10, ACCEL_GROUP(std::vector), 10)
TEST_PT_INSTANCE(Find, Vector100, ACCEL_GROUP(std::vector), 100)
TEST_PT_INSTANCE(Find, Vector1000, ACCEL_GROUP(std::vector), 1000)

//================================================
// Fixtures
//================================================

// Any subset of the member functions below may be provided to implement a fixture.
struct Integers : public accel::Test {
    // Static fields are shared between tests using the same fixture.
    static std::vector<uint64_t> static_integers;

    // This member function is called before any test that uses this fixture is run.
    static void static_set_up() { static_integers = {1, 2, 3}; }
    // This member function is called after all tests that use this fixture have been run.
    static void static_tear_down() { static_integers.clear(); }

    // Non-static fields are not shared between tests using the same fixture.
    std::vector<uint64_t> integers;

    // This member function is called before each test that uses this fixture is run.
    virtual void set_up() override { integers = {4, 5, 6}; }
    // This member function is called after each test that uses this fixture is run.
    virtual void tear_down() override { integers.clear(); }
};

std::vector<uint64_t> Integers::static_integers;

TEST_F(Integers, Test) {
    accel::retain(static_integers);
    accel::retain(integers);
}

//================================================
// Assertions
//================================================

ASSERTION(is_even, uint64_t integer) {
    if (integer % 2 == 0) {
        return PASS;
    } else {
        return FAIL << integer << " is not even";
    }
}

ASSERTION_T(is_equal_within, ACCEL_GROUP(class T), T left, T right, T delta) {
    auto difference = std::max(left, right) - std::min(left, right);
    if (difference <= delta) {
        return PASS;
    } else {
        auto failure = FAIL;
        accel::add_information(failure, "left", left, assertion.arguments[0]);
        accel::add_information(failure, "right", right, assertion.arguments[1]);
        failure.add_information("difference", std::to_string(difference));
        return failure;
    }
}

ASSERTION_GROUP(is_really_even, uint64_t integer) {
    EXPECT(is_even, integer);
    EXPECT(is_even, integer);
}

ASSERTION_GROUP_T(is_really_equal_within, ACCEL_GROUP(class T), T left, T right, T delta) {
    EXPECT(is_equal_within, left, right, delta);
    EXPECT(is_equal_within, left, right, delta);
}

TEST(Custom) {
    EXPECT(is_even, 2);
    EXPECT(is_even, 3);

    EXPECT(is_equal_within, 5 + 5, 15, 5);
    EXPECT(is_equal_within, 5 + 5, 20, 5);

    EXPECT_GROUP(is_really_even, 2);
    EXPECT_GROUP(is_really_even, 3);

    EXPECT_GROUP(is_really_equal_within, 5 + 5, 15, 5);
    EXPECT_GROUP(is_really_equal_within, 5 + 5, 20, 5);
}

TEST(Boolean) {
    EXPECT_TRUE(2 + 2 == 4);
    EXPECT_TRUE(2 + 2 == 5);

    EXPECT_FALSE(2 + 2 == 4);
    EXPECT_FALSE(2 + 2 == 5);
}

TEST(Comparison) {
    EXPECT_EQ(2 + 2, 4);
    EXPECT_EQ(2 + 2, 5);

    EXPECT_NE(2 + 2, 4);
    EXPECT_NE(2 + 2, 5);

    EXPECT_GT(2 + 2, 3);
    EXPECT_GT(2 + 2, 4);

    EXPECT_LT(2 + 2, 5);
    EXPECT_LT(2 + 2, 4);

    EXPECT_GE(2 + 2, 3);
    EXPECT_GE(2 + 2, 4);
    EXPECT_GE(2 + 2, 5);

    EXPECT_LE(2 + 2, 5);
    EXPECT_LE(2 + 2, 4);
    EXPECT_LE(2 + 2, 3);
}

TEST(FloatingPointComparison) {
    EXPECT_FPEQ(3.14159f, 3.141589f, 10);
    EXPECT_FPEQ(3.14159f, 3.141581f, 10);

    EXPECT_FPNE(3.14159f, 3.141589f, 10);
    EXPECT_FPNE(3.14159f, 3.141581f, 10);
}

#if !defined(ACCEL_NO_EXCEPTIONS)
TEST(Exception) {
    EXPECT_THROW(throw std::runtime_error{"Oh no!"});
    EXPECT_THROW();

    EXPECT_NOTHROW(throw std::runtime_error{"Oh no!"});
    EXPECT_NOTHROW(throw int{42});
    EXPECT_NOTHROW();
}
#endif

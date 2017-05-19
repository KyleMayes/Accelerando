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

TEST(Arithmetic) {
    ASSERT_EQ(2 + 2, 4);
    ASSERT_EQ(2 * 2, 4);
}

ASSERTION(is_even, uint64_t integer) {
    if (integer % 2 == 0) {
        return PASS;
    } else {
        return FAIL << integer << " is not even";
    }
}

ASSERTION(is_odd, uint64_t integer) {
    if (integer % 2 != 0) {
        return PASS;
    } else {
        return FAIL << integer << " is not odd";
    }
}

TEST(Parity) {
    ASSERT(is_even, 2);
    ASSERT(is_odd, 3);
    ASSERT(is_even, 4);
    ASSERT(is_odd, 5);
}

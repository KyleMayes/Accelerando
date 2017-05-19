# accelerando

A C++17 testing and benchmarking library.

Released under the Apache License 2.0.

[Documentation](https://github.com/KyleMayes/accelerando/wiki/Documentation)

## Features

* Powerful custom assertions
* Parameterized benchmarks and tests
* Templated benchmarks and tests
* Exceptions and RTTI not required

## Example

```c++
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
```

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

#ifndef ACCEL_ASSERT_HPP
#define ACCEL_ASSERT_HPP

#include <accelerando/map.hpp>
#include <accelerando/registry.hpp>

#include <array>
#include <cstring>
#include <functional>
#include <iomanip>

namespace accel {

/// A collection of information about the assertion calling an assertion function.
template <size_t SIZE>
struct Assertion {
    /// The location of the assertion.
    Location location;
    /// The string representation of the assertion.
    const char* assertion;
    /// The string representations of the arguments supplied by the assertion.
    std::array<const char*, SIZE> arguments;

    /// Constructs a collection of information about an assertion.
    Assertion(Location location, const char* assertion, std::array<const char*, SIZE> arguments)
        : location{location}, assertion{assertion}, arguments{arguments} { }
};

/// Assists `ACCEL_SIZEOF`.
#define ACCEL_SIZEOF_HELPER(_) + 1
/// Expands to the number of supplied arguments.
#define ACCEL_SIZEOF(...) (0 ACCEL_MAP(ACCEL_SIZEOF_HELPER, __VA_ARGS__))

/// Defines a templated assertion function.
#define ASSERTION_T(FUNCTION, TYPES, ...) \
    template <TYPES> \
    std::optional<::accel::Failure> \
    FUNCTION(::accel::Assertion<ACCEL_SIZEOF(__VA_ARGS__)> assertion, __VA_ARGS__)

/// Defines an assertion function.
#define ASSERTION(FUNCTION, ...) \
    ASSERTION_T(FUNCTION, ACCEL_GROUP(class = void), __VA_ARGS__)

/// Defines a templated assertion group function.
#define ASSERTION_GROUP_T(FUNCTION, TYPES, ...) \
    template <TYPES> \
    void FUNCTION(ACCEL_UNUSED std::vector<::accel::Failure>& _Accel_failures, __VA_ARGS__)

/// Defines an assertion group function.
#define ASSERTION_GROUP(FUNCTION, ...) \
    ASSERTION_GROUP_T(FUNCTION, ACCEL_GROUP(class = void), __VA_ARGS__)

/// Expands to a value which is returned by successful assertion functions.
#define PASS (std::optional<::accel::Failure>{})
/// Expands to a value which is returned by non-successful assertion functions.
#define FAIL (::accel::Failure{assertion.location, assertion.assertion})

namespace detail {
    std::optional<std::string> stringify(bool value);
    std::optional<std::string> stringify(char value);
    std::optional<std::string> stringify(const char* value);
    std::optional<std::string> stringify(const std::string& value);

    template <class T, std::enable_if_t<std::is_integral_v<T>, int> = 0>
    std::optional<std::string> stringify(T value) {
        return {std::to_string(value)};
    }

    template <class T, std::enable_if_t<std::is_floating_point_v<T>, int> = 0>
    std::optional<std::string> stringify(T value) {
        std::stringstream ss;
        ss << std::setprecision(std::is_same_v<T, float> ? 9 : 17);
        ss << value;
        return {ss.str()};
    }

    template <class T, std::enable_if_t<std::is_pointer_v<T>, int> = 0>
    std::optional<std::string> stringify(T value) {
        std::stringstream ss;
        ss << "0x" << std::hex << std::uppercase;
        ss << std::setfill('0') << std::setw(sizeof(T) == 4 ? 8 : 16);
        ss << reinterpret_cast<uintptr_t>(value);
        return {ss.str()};
    }

    template <class T, std::enable_if_t<std::is_enum_v<T>, int> = 0>
    std::optional<std::string> stringify(T value) {
        return stringify(static_cast<std::underlying_type_t<T>>(value));
    }

    template <class T>
    static auto test_stringstream(int)
        -> decltype(std::declval<std::stringstream&>() << std::declval<const T&>(), int());

    template <class T>
    static void test_stringstream(bool);

    template <class T, std::enable_if_t<!std::is_scalar_v<T>, int> = 0>
    std::optional<std::string> stringify(const T& value) {
        if constexpr (std::is_same_v<decltype(test_stringstream<T>(0)), int>) {
            std::stringstream ss;
            ss << value;
            return {ss.str()};
        } else {
            return {};
        }
    }
}

/// Returns a string representation of the supplied value.
template <class T>
std::optional<std::string> stringify(const T& value) {
    return detail::stringify(value);
}

/// Adds a key-value pair to the supplied assertion failure if the string representation of the
/// supplied value differs from the supplied argument string representation.
template <class T>
void add_information(Failure& failure, const char* key, const T& value, const char* argument) {
    if (auto string = stringify(value); string && *string != argument) {
        failure.add_information(key, std::move(*string));
    }
}

namespace detail {
    ASSERTION_T(boolean,
    ACCEL_GROUP(class T), const T& value, bool expected) {
        if (value == expected) {
            return PASS;
        } else {
            auto failure = FAIL;
            add_information(failure, "value", value, assertion.arguments[0]);
            return failure;
        }
    }

    ASSERTION_T(cmp,
    ACCEL_GROUP(class T, class U, class C), const T& left, const U& right, C cmp) {
        if (cmp(left, right)) {
            return PASS;
        } else {
            auto failure = FAIL;
            add_information(failure, "left", left, assertion.arguments[0]);
            add_information(failure, "right", right, assertion.arguments[1]);
            return failure;
        }
    }

    /// Returns the difference in units in the last place for the supplied floats.
    uint64_t get_ulp_difference(float left, float right);
    /// Returns the difference in units in the last place for the supplied doubles.
    uint64_t get_ulp_difference(double left, double right);

    ASSERTION_T(fpcmp,
    ACCEL_GROUP(class T,  class F), T left, T right, uint64_t ulp, F f) {
        auto difference = get_ulp_difference(left, right);
        if (f(difference, ulp)) {
            return PASS;
        } else {
            auto failure = FAIL;
            add_information(failure, "left", left, assertion.arguments[0]);
            add_information(failure, "right", right, assertion.arguments[1]);
            failure.add_information("difference", std::to_string(difference));
            return failure;
        }
    }

#if !defined(ACCEL_NO_EXCEPTIONS)
    ASSERTION_T(exception,
    ACCEL_GROUP(class E), E expression, bool expected) {
        try {
            expression();
            if (!expected) {
                return PASS;
            } else {
                return FAIL << "Expected exception";
            }
        } catch (const std::runtime_error& e) {
            if (expected) {
                return PASS;
            } else {
                auto failure = FAIL << "Unexpected std::runtime_error";
                failure.add_information("what()", *stringify(e.what()));
                return failure;
            }
        } catch (...) {
            if (expected) {
                return PASS;
            } else {
                return FAIL << "Unexpected exception of unknown type";
            }
        }
    }
#endif
}

//================================================
// Custom
//================================================

/// Expands to the stringified version of the supplied argument.
#define ACCEL_STRINGIFY(VALUE) #VALUE,

/// Assists `ACCEL_ASSERT`.
#define ACCEL_ASSERT_HELPER(RETURN, ASSERTION, FUNCTION, ...) { \
    constexpr auto SIZE = ACCEL_SIZEOF(__VA_ARGS__); \
    ::accel::Location location{__FILE__, __LINE__}; \
    std::array<const char*, SIZE> arguments{{ACCEL_MAP(ACCEL_STRINGIFY, __VA_ARGS__)}}; \
    ::accel::Assertion<SIZE> assertion{location, ASSERTION, arguments}; \
    if (auto failure = FUNCTION(assertion, __VA_ARGS__); failure) { \
        _Accel_failures.push_back(std::move(*failure)); \
        if (RETURN) { \
            return; \
        } \
    } \
}

/// Defines a custom assertion.
#define ACCEL_ASSERT(RETURN, NAME, FUNCTION, ...) \
    ACCEL_ASSERT_HELPER(RETURN, NAME "(" #FUNCTION ", " #__VA_ARGS__ ")", FUNCTION, __VA_ARGS__)

/// Defines a terminating custom assertion.
#define ASSERT(FUNCTION, ...) ACCEL_ASSERT(true, "ASSERT", FUNCTION, __VA_ARGS__)
/// Defines a non-terminating custom assertion.
#define EXPECT(FUNCTION, ...) ACCEL_ASSERT(false, "EXPECT", FUNCTION, __VA_ARGS__)

/// Defines a custom group assertion.
#define ACCEL_ASSERT_GROUP(RETURN, FUNCTION, ...) { \
    std::vector<::accel::Failure> failures; \
    FUNCTION(failures, __VA_ARGS__); \
    auto failed = !failures.empty(); \
    for (auto failure : failures) { \
        failure.stack.insert(failure.stack.begin(), ::accel::Location{__FILE__, __LINE__}); \
        _Accel_failures.push_back(std::move(failure)); \
    } \
    if (failed && RETURN) { \
        return; \
    } \
}

/// Defines a terminating custom group assertion.
#define ASSERT_GROUP(FUNCTION, ...) ACCEL_ASSERT_GROUP(true, FUNCTION, __VA_ARGS__)
/// Defines a non-terminating custom group assertion.
#define EXPECT_GROUP(FUNCTION, ...) ACCEL_ASSERT_GROUP(false, FUNCTION, __VA_ARGS__)

//================================================
// Boolean
//================================================

/// Defines a boolean assertion.
#define ACCEL_ASSERT_BOOLEAN(RETURN, NAME, VALUE, EXPECTED) \
    ACCEL_ASSERT_HELPER(RETURN, NAME "(" #VALUE ")", \
        ::accel::detail::boolean, VALUE, EXPECTED)

/// Defines a terminating truth assertion.
#define ASSERT_TRUE(VALUE) ACCEL_ASSERT_BOOLEAN(true, "ASSERT_TRUE", VALUE, true)
/// Defines a non-terminating truth assertion.
#define EXPECT_TRUE(VALUE) ACCEL_ASSERT_BOOLEAN(false, "EXPECT_TRUE", VALUE, true)

/// Defines a terminating falsity assertion.
#define ASSERT_FALSE(VALUE) ACCEL_ASSERT_BOOLEAN(true, "ASSERT_FALSE", VALUE, false)
/// Defines a non-terminating falsity assertion.
#define EXPECT_FALSE(VALUE) ACCEL_ASSERT_BOOLEAN(false, "EXPECT_FALSE", VALUE, false)

//================================================
// Comparison
//================================================

/// Defines a comparison assertion.
#define ACCEL_ASSERT_CMP(RETURN, NAME, LEFT, RIGHT, F) \
    ACCEL_ASSERT_HELPER(RETURN, NAME "(" #LEFT ", " #RIGHT ")", \
        ::accel::detail::cmp, LEFT, RIGHT, F{})

/// Defines a terminating equality assertion.
#define ASSERT_EQ(LEFT, RIGHT) ACCEL_ASSERT_CMP(true, "ASSERT_EQ", LEFT, RIGHT, std::equal_to)
/// Defines a non-terminating equality assertion.
#define EXPECT_EQ(LEFT, RIGHT) ACCEL_ASSERT_CMP(false, "EXPECT_EQ", LEFT, RIGHT, std::equal_to)

/// Defines a terminating inequality assertion.
#define ASSERT_NE(LEFT, RIGHT) ACCEL_ASSERT_CMP(true, "ASSERT_NE", LEFT, RIGHT, std::not_equal_to)
/// Defines a non-terminating inequality assertion.
#define EXPECT_NE(LEFT, RIGHT) ACCEL_ASSERT_CMP(false, "EXPECT_NE", LEFT, RIGHT, std::not_equal_to)

/// Defines a terminating greater than assertion.
#define ASSERT_GT(LEFT, RIGHT) ACCEL_ASSERT_CMP(true, "ASSERT_GT", LEFT, RIGHT, std::greater)
/// Defines a non-terminating greater than assertion.
#define EXPECT_GT(LEFT, RIGHT) ACCEL_ASSERT_CMP(false, "EXPECT_GT", LEFT, RIGHT, std::greater)

/// Defines a terminating less than assertion.
#define ASSERT_LT(LEFT, RIGHT) ACCEL_ASSERT_CMP(true, "ASSERT_LT", LEFT, RIGHT, std::less)
/// Defines a non-terminating less than assertion.
#define EXPECT_LT(LEFT, RIGHT) ACCEL_ASSERT_CMP(false, "EXPECT_LT", LEFT, RIGHT, std::less)

/// Defines a terminating greater than or equal assertion.
#define ASSERT_GE(LEFT, RIGHT) ACCEL_ASSERT_CMP(true, "ASSERT_GE", LEFT, RIGHT, std::greater_equal)
/// Defines a non-terminating greater than or equal assertion.
#define EXPECT_GE(LEFT, RIGHT) ACCEL_ASSERT_CMP(false, "EXPECT_GE", LEFT, RIGHT, std::greater_equal)

/// Defines a terminating less than or equal assertion.
#define ASSERT_LE(LEFT, RIGHT) ACCEL_ASSERT_CMP(true, "ASSERT_LE", LEFT, RIGHT, std::less_equal)
/// Defines a non-terminating less than or equal assertion.
#define EXPECT_LE(LEFT, RIGHT) ACCEL_ASSERT_CMP(false, "EXPECT_LE", LEFT, RIGHT, std::less_equal)

// Floating-point --------------------------------

/// Defines a floating-point comparison assertion.
#define ACCEL_ASSERT_FPCMP(RETURN, NAME, LEFT, RIGHT, ULP, F) \
    ACCEL_ASSERT_HELPER(RETURN, NAME "(" #LEFT ", " #RIGHT "," #ULP ")", \
        ::accel::detail::fpcmp, LEFT, RIGHT, ULP, F{})

/// Defines a terminating floating-point equality assertion.
#define ASSERT_FPEQ(LEFT, RIGHT, ULP) \
    ACCEL_ASSERT_FPCMP(true, "ASSERT_FPEQ", LEFT, RIGHT, ULP, std::less_equal)
/// Defines a non-terminating floating-point equality assertion.
#define EXPECT_FPEQ(LEFT, RIGHT, ULP) \
    ACCEL_ASSERT_FPCMP(false, "EXPECT_FPEQ", LEFT, RIGHT, ULP, std::less_equal)

/// Defines a terminating floating-point inequality assertion.
#define ASSERT_FPNE(LEFT, RIGHT, ULP) \
    ACCEL_ASSERT_FPCMP(true, "ASSERT_FPNE", LEFT, RIGHT, ULP, std::greater)
/// Defines a non-terminating inequality assertion.
#define EXPECT_FPNE(LEFT, RIGHT, ULP) \
    ACCEL_ASSERT_FPCMP(false, "EXPECT_FPNE", LEFT, RIGHT, ULP, std::greater)

//================================================
// Exception
//================================================

#if !defined(ACCEL_NO_EXCEPTIONS)
/// Defines an exception assertion.
#define ACCEL_ASSERT_THROW(RETURN, NAME, EXPRESSION, EXPECTED) \
    ACCEL_ASSERT_HELPER(RETURN, NAME "(" #EXPRESSION ")", \
        ::accel::detail::exception, [&] { EXPRESSION; }, EXPECTED)

/// Defines a terminating throw assertion.
#define ASSERT_THROW(EXPRESSION) ACCEL_ASSERT_THROW(true, "ASSERT_THROW", EXPRESSION, true)
/// Defines a non-terminating throw assertion.
#define EXPECT_THROW(EXPRESSION) ACCEL_ASSERT_THROW(false, "EXPECT_THROW", EXPRESSION, true)

/// Defines a terminating nothrow assertion.
#define ASSERT_NOTHROW(EXPRESSION) ACCEL_ASSERT_THROW(true, "ASSERT_NOTHROW", EXPRESSION, false)
/// Defines a non-terminating nothrow assertion.
#define EXPECT_NOTHROW(EXPRESSION) ACCEL_ASSERT_THROW(false, "EXPECT_NOTHROW", EXPRESSION, false)
#endif

}

#endif

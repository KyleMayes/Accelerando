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

#ifndef ACCEL_REGISTRY_HPP
#define ACCEL_REGISTRY_HPP

#include <accelerando/benchmark.hpp>
#include <accelerando/test.hpp>

#include <memory>
#include <type_traits>

namespace accel {

/// A pointer to a nullary function that returns nothing.
using Function = std::add_pointer_t<void(void)>;

/// A pair of static lifecycle functions provided by a benchmark or test class.
struct Lifecycle {
    /// The initialization static lifecycle function.
    Function set_up;
    /// The termination static lifecycle function.
    Function tear_down;

    /// Constructs a pair of static lifecycle functions.
    Lifecycle(Function set_up, Function tear_down);

    friend bool operator<(Lifecycle left, Lifecycle right);
};

/// A benchmark or test instance.
template <class T>
struct Instance {
    /// The static lifecycle functions.
    Lifecycle lifecycle;
    /// The user-supplied name.
    std::string name;
    /// The instance.
    std::unique_ptr<T> instance;

    /// Constructs an instance.
    Instance(Lifecycle lifecycle, std::string name, std::unique_ptr<T> instance)
        : lifecycle{lifecycle}, name{std::move(name)}, instance{std::move(instance)} { }
};

/// A collection of registered benchmarks or tests.
class Registry {
    std::vector<Instance<Benchmark>> benchmarks;
    std::vector<Instance<Test>> tests;

public:
    /// Returns the registry.
    static Registry& get();

    /// Registers the benchmark provided as a type parameter under the supplied name.
    template <class T>
    int register_benchmark(const char* name) {
        Lifecycle lifecycle{&T::static_set_up, &T::static_tear_down};
        benchmarks.emplace_back(lifecycle, name, std::make_unique<T>());
        return 0;
    }

    /// Registers the test provided as a type parameter under the supplied name.
    template <class T>
    int register_test(const char* name, Location location) {
        Lifecycle lifecycle{&T::static_set_up, &T::static_tear_down};
        tests.emplace_back(lifecycle, name, std::make_unique<T>());
        tests.back().instance->location = location;
        return 0;
    }

    /// Returns the registered benchmarks.
    const std::vector<Instance<Benchmark>>& get_benchmarks() const;
    /// Returns the registered tests.
    const std::vector<Instance<Test>>& get_tests() const;

private:
    Registry() = default;
};

/// Assists `ACCEL_CLASS`.
#define ACCEL_CLASS_HELPER(NAME) _Accel_##NAME
/// Expands to a benchmark or test class name.
#define ACCEL_CLASS(NAME) ACCEL_CLASS_HELPER(NAME)

/// Assists `ACCEL_PASTE`.
#define ACCEL_PASTE_HELPER(A, B) A##B
/// Expands to the concatenation of the two arguments.
#define ACCEL_PASTE(A, B) ACCEL_PASTE_HELPER(A, B)

/// Expands to the arguments to emulate the usage of multiple parameter groups.
#define ACCEL_GROUP(...) __VA_ARGS__

/// A unique identifier.
#define ACCEL_UNIQUE ACCEL_PASTE(_Accel, __COUNTER__)

#if defined(__GNUC__)
    /// An attribute which marks a variable as unused.
    #define ACCEL_UNUSED [[gnu::unused]]
#else
    /// An attribute which marks a variable as unused.
    #define ACCEL_UNUSED
#endif

//================================================
// Benchmarks
//================================================

/// Implements `BENCHMARK_F`.
#define ACCEL_BENCHMARK_F(FIXTURE, NAME) \
    class ACCEL_CLASS(NAME) : public FIXTURE { \
    protected: \
        virtual void execute() override final; \
    }; \
    auto ACCEL_UNIQUE = ::accel::Registry::get() \
        .register_benchmark<ACCEL_CLASS(NAME)>(#NAME); \
    void ACCEL_CLASS(NAME)::execute()

/// Defines and registers a benchmark.
#define BENCHMARK_F(FIXTURE, NAME) \
    ACCEL_BENCHMARK_F(FIXTURE, NAME)

/// Defines and registers a benchmark.
#define BENCHMARK(NAME) \
    BENCHMARK_F(::accel::Benchmark, NAME)

/// Defines a parameterized and templated benchmark.
#define BENCHMARK_PT_F(FIXTURE, NAME, TYPES, ...) \
    class ACCEL_CLASS(NAME) : public FIXTURE { \
    protected: \
        template <TYPES> \
        void execute_pt(__VA_ARGS__); \
    }; \
    template <TYPES> \
    void ACCEL_CLASS(NAME)::execute_pt(__VA_ARGS__)

/// Defines a parameterized and templated benchmark.
#define BENCHMARK_PT(NAME, TYPES, ...) \
    BENCHMARK_PT_F(::accel::Benchmark, NAME, ACCEL_GROUP(TYPES), __VA_ARGS__)

/// Defines and registers an instance of a parameterized and templated benchmark.
#define BENCHMARK_PT_INSTANCE(NAME, SUBNAME, TYPES, ...) \
    ACCEL_BENCHMARK_F(ACCEL_CLASS(NAME), NAME##_##SUBNAME) { \
        execute_pt<TYPES>(__VA_ARGS__); \
    }

/// Defines a parameterized benchmark.
#define BENCHMARK_P_F(FIXTURE, NAME, ...) \
    BENCHMARK_PT_F(FIXTURE, NAME, class, __VA_ARGS__)

/// Defines a parameterized benchmark.
#define BENCHMARK_P(NAME, ...) \
    BENCHMARK_P_F(::accel::Benchmark, NAME, __VA_ARGS__)

/// Defines and registers an instance of a parameterized benchmark.
#define BENCHMARK_P_INSTANCE(NAME, SUBNAME, ...) \
    BENCHMARK_PT_INSTANCE(NAME, SUBNAME, void, __VA_ARGS__)

/// Defines a templated benchmark.
#define BENCHMARK_T_F(FIXTURE, NAME, ...) \
    BENCHMARK_PT_F(FIXTURE, NAME, ACCEL_GROUP(__VA_ARGS__), )

/// Defines a templated benchmark.
#define BENCHMARK_T(NAME, ...) \
    BENCHMARK_T_F(::accel::Benchmark, NAME, __VA_ARGS__)

/// Defines and registers an instance of a templated benchmark.
#define BENCHMARK_T_INSTANCE(NAME, SUBNAME, ...) \
    BENCHMARK_PT_INSTANCE(NAME, SUBNAME, ACCEL_GROUP(__VA_ARGS__), )

//================================================
// Tests
//================================================

/// Implements `TEST_F`.
#define ACCEL_TEST_F(FIXTURE, NAME) \
    class ACCEL_CLASS(NAME) : public FIXTURE { \
    protected: \
        virtual void execute(std::vector<::accel::Failure>& _Accel_failures) override final; \
    }; \
    auto ACCEL_UNIQUE = ::accel::Registry::get() \
        .register_test<ACCEL_CLASS(NAME)>(#NAME, ::accel::Location{__FILE__, __LINE__}); \
    void ACCEL_CLASS(NAME)::execute(ACCEL_UNUSED std::vector<::accel::Failure>& _Accel_failures)

/// Defines and registers a test.
#define TEST_F(FIXTURE, NAME) \
    ACCEL_TEST_F(FIXTURE, NAME)

/// Defines and registers a test.
#define TEST(NAME) \
    TEST_F(::accel::Test, NAME)

/// Defines a parameterized and templated test.
#define TEST_PT_F(FIXTURE, NAME, TYPES, ...) \
    class ACCEL_CLASS(NAME) : public FIXTURE { \
    protected: \
        template <TYPES> \
        void execute_pt(__VA_ARGS__); \
    }; \
    template <TYPES> \
    void ACCEL_CLASS(NAME)::execute_pt(__VA_ARGS__)

/// Defines a parameterized and templated test.
#define TEST_PT(NAME, TYPES, ...) \
    TEST_PT_F(::accel::Test, NAME, ACCEL_GROUP(TYPES), __VA_ARGS__)

/// Defines and registers an instance of a parameterized and templated test.
#define TEST_PT_INSTANCE(NAME, SUBNAME, TYPES, ...) \
    ACCEL_TEST_F(ACCEL_CLASS(NAME), NAME##_##SUBNAME) { \
        execute_pt<TYPES>(__VA_ARGS__); \
    }

/// Defines a parameterized test.
#define TEST_P_F(FIXTURE, NAME, ...) \
    TEST_PT_F(FIXTURE, NAME, class, __VA_ARGS__)

/// Defines a parameterized test.
#define TEST_P(NAME, ...) \
    TEST_P_F(::accel::Test, NAME, __VA_ARGS__)

/// Defines and registers an instance of a parameterized test.
#define TEST_P_INSTANCE(NAME, SUBNAME, ...) \
    TEST_PT_INSTANCE(NAME, SUBNAME, void, __VA_ARGS__)

/// Defines a templated test.
#define TEST_T_F(FIXTURE, NAME, ...) \
    TEST_PT_F(FIXTURE, NAME, ACCEL_GROUP(__VA_ARGS__), )

/// Defines a templated test.
#define TEST_T(NAME, ...) \
    TEST_T_F(::accel::Test, NAME, __VA_ARGS__)

/// Defines and registers an instance of a templated test.
#define TEST_T_INSTANCE(NAME, SUBNAME, ...) \
    TEST_PT_INSTANCE(NAME, SUBNAME, ACCEL_GROUP(__VA_ARGS__), )

}

#endif

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

#include <accelerando/main.hpp>

#include <accelerando/registry.hpp>

#include <iomanip>
#include <iostream>
#include <regex>

#if defined(_WIN32)
#include <Windows.h>
#endif

namespace accel {

#if defined(_WIN32)
/// Prints colored text to the console.
struct Color {
    WORD color;

    constexpr Color(WORD color) : color{color} { }

    template <class T>
    void print(const T& value) const {
        auto handle = GetStdHandle(STD_OUTPUT_HANDLE);

        // Save the current console attributes and set the console text color.
        CONSOLE_SCREEN_BUFFER_INFO info;
        GetConsoleScreenBufferInfo(handle, &info);
        auto attributes = info.wAttributes;
        SetConsoleTextAttribute(handle, color);

        std::cout << value;

        // Reset the console attributes.
        SetConsoleTextAttribute(handle, attributes);
    }
};

constexpr static Color RED = Color{FOREGROUND_RED};
constexpr static Color GREEN = Color{FOREGROUND_GREEN};
constexpr static Color BLUE = Color{FOREGROUND_BLUE};
constexpr static Color YELLOW = Color{FOREGROUND_RED | FOREGROUND_GREEN};
constexpr static Color MAGENTA = Color{FOREGROUND_RED | FOREGROUND_BLUE};
constexpr static Color CYAN = Color{FOREGROUND_GREEN | FOREGROUND_BLUE};
#else
/// Prints colored text to the console.
struct Color {
    const char* color;

    constexpr Color(const char* color) : color{color} { }

    template <class T>
    void print(const T& value) const {
        std::cout << color << value << "\x1B[0m";
    }
};

constexpr static Color RED = Color{"\x1B[31m"};
constexpr static Color GREEN = Color{"\x1B[32m"};
constexpr static Color BLUE = Color{"\x1B[34m"};
constexpr static Color YELLOW = Color{"\x1B[33m"};
constexpr static Color MAGENTA = Color{"\x1B[35m"};
constexpr static Color CYAN = Color{"\x1B[36m"};
#endif

/// Stores and parses command-line arguments.
struct Options {
    Nanoseconds<uint64_t> limit{5'000'000'000};
    std::regex regex{".*"};

    Options() = default;

    void print_help(char* name, bool benchmarks) {
        std::printf("Usage: %s [options]\n\nOptions:\n", name);
        if (benchmarks) {
            std::printf("  --limit=<number>      Set the benchmark time limit (seconds)\n");
            std::printf("  --regex=<regex>       Set the benchmark filter\n");
        } else {
            std::printf("  --regex=<regex>       Set the test filter\n");
        }
    }

    bool parse_limit(const std::string& value) {
        char* end;
        auto number = std::strtod(value.data(), &end);
        if (end == value.data() + value.size()) {
            limit = Nanoseconds<uint64_t>{static_cast<uint64_t>(1'000'000'000 * number)};
            return true;
        } else {
            RED.print("ERROR: ");
            std::cout << "invalid number: '" << value << "'" << std::endl;
            return false;
        }
    }

    bool parse_regex(const std::string& value) {
    #if defined(ACCEL_NO_EXCEPTIONS)
        regex = std::regex{value};
        return true;
    #else
        try {
            regex = std::regex{value};
            return true;
        } catch (const std::regex_error&) {
            RED.print("ERROR: ");
            std::cout << "invalid regex: '" << value << "'" << std::endl;
            return false;
        }
    #endif
    }

    std::optional<int> parse(int argc, char* argv[], bool benchmarks) {
        for (int index = 1; index < argc; ++index) {
            std::string argument{argv[index]};
            if (argument == "--help") {
                print_help(argv[0], benchmarks);
                return {0};
            } else if (benchmarks && argument.compare(0, 8, "--limit=") == 0) {
                if (!parse_limit(argument.substr(8))) {
                    return {1};
                }
            } else if (argument.compare(0, 8, "--regex=") == 0) {
                if (!parse_regex(argument.substr(8))) {
                    return {1};
                }
            } else {
                RED.print("ERROR: ");
                std::cout << "invalid argument: '" << argument << "'" << std::endl;
                return {1};
            }
        }
        return {};
    }
};

std::string format_nanoseconds(double nanoseconds, size_t length) {
    std::pair<double, const char*> display;
    if (nanoseconds < 1'000.0) {
        display = {nanoseconds, " ns"};
    }
    else if (nanoseconds < 1'000'000.0) {
        display = {nanoseconds / 1000.0, " µs"};
    }
    else if (nanoseconds < 1'000'000'000.0) {
        display = {nanoseconds / 1'000'000.0, " ms"};
    }
    else {
        display = {nanoseconds / 1'000'000'000.0, " s"};
    }

    std::stringstream ss;
    ss << std::setfill('0') << std::setw(length) << std::left << display.first;
    return ss.str().substr(0, length) + display.second;
}

template <class T>
struct Runner;

template <>
struct Runner<Benchmark> {
    Runner() = default;

    void handle_start(const std::vector<const Instance<Benchmark>*>& filtered) {
        GREEN.print("╔════════════╗ ");
        MAGENTA.print(std::to_string(filtered.size()) + " benchmark(s).\n");
        if (!filtered.empty()) {
            std::cout << std::endl;
        }
    }

    int handle_end() {
        GREEN.print("\n╚════════════╝ ");
        MAGENTA.print("All benchmarks completed.\n");
        return 0;
    }

    void handle_instance(const Instance<Benchmark>& benchmark, const Options& options) {
        GREEN.print("┌─RUN────────┐ ");
        CYAN.print(benchmark.name);
        std::cout << std::endl;

        auto report = benchmark.instance->run(options.limit);
        BLUE.print(" t: ");
        std::cout << format_nanoseconds(report.ols.b1.count(), 6) << std::endl;
        std::printf("    %.4f R²\n", report.ols.r2);
        BLUE.print(" μ: ");
        std::cout << format_nanoseconds(report.mean.count(), 6) << std::endl;
        BLUE.print(" σ: ");
        std::cout << format_nanoseconds(report.stddev.count(), 6) << std::endl;

        GREEN.print("└───────DONE─┘ ");
        CYAN.print(benchmark.name);
        std::cout << std::endl;
    }
};

template <class T>
int run(int argc, char* argv[], const std::vector<Instance<T>>& instances) {
    // Parse the command-line arguments.
    Options options;
    if (auto code = options.parse(argc, argv, std::is_same_v<T, Benchmark>); code) {
        return *code;
    }

    // Collect the filtered instances.
    std::vector<const Instance<T>*> filtered;
    std::map<Lifecycle, std::pair<size_t, size_t>> lifecycles;
    for (const auto& instance : instances) {
        if (std::regex_match(instance.name, options.regex)) {
            filtered.push_back(&instance);
            if (auto iterator = lifecycles.find(instance.lifecycle); iterator != lifecycles.end()) {
                iterator->second.second += 1;
            } else {
                lifecycles.emplace(instance.lifecycle, std::make_pair(0, 1));
            }
        }
    }

    // Run the filtered instances.
    Runner<T> runner;
    runner.handle_start(filtered);
    for (const auto& instance : filtered) {
        // Run the static initialization lifecycle function if necessary.
        auto iterator = lifecycles.find(instance->lifecycle);
        iterator->second.first += 1;
        if (iterator->second.first == 1) {
            iterator->first.set_up();
        }

        // Run the instance.
        instance->instance->set_up();
        runner.handle_instance(*instance, options);
        instance->instance->tear_down();

        // Run the static termination lifecycle function if necessary.
        if (iterator->second.first == iterator->second.second) {
            iterator->first.tear_down();
        }
    }
    return runner.handle_end();
}

int main_benchmarks(int argc, char* argv[]) {
    return run(argc, argv, Registry::get().get_benchmarks());
}

int main_tests(int argc, char* argv[]) {
    return 0;
}

}

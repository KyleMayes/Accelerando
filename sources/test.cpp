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

#include <accelerando/test.hpp>

#include <stdexcept>

namespace accel {

Location::Location(const char* file, uint64_t line) : file{file}, line{line} { }

Failure::Failure(Location location, std::string assertion)
    : location{location}
    , assertion{std::move(assertion)} { }

void Failure::add_information(std::string key, std::string value) {
    information.emplace_back(std::move(key), std::move(value));
}

TestReport::TestReport(std::vector<Failure> failures) : failures{std::move(failures)} { }

TestReport Test::run() {
    std::vector<Failure> failures;

    set_up();
#if defined(ACCEL_NO_EXCEPTIONS)
    execute(failures);
#else
    try {
        execute(failures);
    } catch (const std::exception& e) {
        failures.emplace_back(location, "Unexpected exception.");
        failures.back().information.emplace_back("message", e.what());
    } catch (...) {
        failures.emplace_back(location, "Unexpected exception of unknown type.");
    }
#endif
    tear_down();

    return {std::move(failures)};
}

}

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

#ifndef ACCEL_MAIN_HPP
#define ACCEL_MAIN_HPP

namespace accel {

/// Runs the registered benchmarks.
int main_benchmarks(int argc, char* argv[]);
/// Runs the registered tests.
int main_tests(int argc, char* argv[]);

/// Defines an implementation of `main` that runs any defined benchmarks.
#define ACCEL_BENCHMARKS \
    int main(int argc, char* argv[]) { \
        return ::accel::main_benchmarks(argc, argv); \
    }

/// Defines an implementation of `main` that runs any defined tests.
#define ACCEL_TESTS \
    int main(int argc, char* argv[]) { \
        return ::accel::main_tests(argc, argv); \
    }

}

#endif

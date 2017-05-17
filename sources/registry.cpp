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

#include <accelerando/registry.hpp>

namespace accel {

Lifecycle::Lifecycle(Function set_up, Function tear_down) : set_up{set_up}, tear_down{tear_down} { }

bool operator<(Lifecycle left, Lifecycle right) {
    if (left.set_up == right.set_up) {
        return left.tear_down < right.tear_down;
    } else {
        return left.set_up < right.set_up;
    }
}

Registry& Registry::get() {
    static Registry instance;
    return instance;
}

const std::vector<Instance<Benchmark>>& Registry::get_benchmarks() const {
    return benchmarks;
}

const std::vector<Instance<Test>>& Registry::get_tests() const {
    return tests;
}

}

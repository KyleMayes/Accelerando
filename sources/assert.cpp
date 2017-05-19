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

#include <accelerando/assert.hpp>

#include <algorithm>
#include <cmath>

namespace accel {

namespace detail {
    std::optional<std::string> stringify(bool value) {
        if (value) {
            return {"true"};
        } else {
            return {"false"};
        }
    }

    std::optional<std::string> stringify(char value) {
        std::stringstream ss;
        if (std::isprint(value)) {
            ss << "'" << value << "'";
        } else {
            ss << "\\x" << std::hex << std::uppercase;
            ss << std::setfill('0') << std::setw(2);
            ss << static_cast<uintptr_t>(value);
        }
        return {ss.str()};
    }

    std::optional<std::string> stringify(const char* value, size_t size) {
        std::string escaped;
        escaped.reserve(size + 2);
        escaped.push_back('"');
        for (size_t index = 0; index < size; ++index) {
            if (std::iscntrl(value[index])) {
                escaped.push_back('\\');
                switch (value[index]) {
                case '"':
                    escaped.push_back('"');
                    break;
                case '\\':
                    escaped.push_back('\\');
                    break;
                case '\b':
                    escaped.push_back('b');
                    break;
                case '\f':
                    escaped.push_back('f');
                    break;
                case '\n':
                    escaped.push_back('n');
                    break;
                case '\r':
                    escaped.push_back('r');
                    break;
                case '\t':
                    escaped.push_back('t');
                    break;
                default:
                    std::stringstream ss;
                    ss << "x" << std::uppercase << std::hex << std::setfill('0') << std::setw(2);
                    ss << static_cast<int>(value[index]);
                    escaped.append(ss.str());
                    break;
                }
            }
            else {
                escaped.push_back(value[index]);
            }
        }
        escaped.push_back('"');
        return escaped;
    }

    std::optional<std::string> stringify(const char* value) {
        return stringify(value, std::char_traits<char>::length(value));
    }

    std::optional<std::string> stringify(const std::string& value) {
        return stringify(value.data(), value.size());
    }

    template <class T, class I>
    union Representation {
        T value;
        I bits;

        Representation(T value) : value{value} { }
    };

    template <class T, class I>
    uint64_t get_ulp_difference(T left, T right) {
        Representation<T, I> l{left}, r{right};
        return std::max(l.bits, r.bits) - std::min(l.bits, r.bits);
    }

    uint64_t get_ulp_difference(float left, float right) {
        return get_ulp_difference<float, uint32_t>(left, right);
    }

    uint64_t get_ulp_difference(double left, double right) {
        return get_ulp_difference<double, uint64_t>(left, right);
    }
}

}

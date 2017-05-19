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

#ifndef ACCEL_MAP_HPP
#define ACCEL_MAP_HPP

// The implementation of `ACCEL_MAP()` below is based on the implementation of `MAP()` in William
// Swanson's `map-macro` library (https://github.com/swansontec/map-macro).

#define ACCEL_EVAL0(...) __VA_ARGS__
#define ACCEL_EVAL1(...) ACCEL_EVAL0(ACCEL_EVAL0(ACCEL_EVAL0(__VA_ARGS__)))
#define ACCEL_EVAL2(...) ACCEL_EVAL1(ACCEL_EVAL1(ACCEL_EVAL1(__VA_ARGS__)))
#define ACCEL_EVAL3(...) ACCEL_EVAL2(ACCEL_EVAL2(ACCEL_EVAL2(__VA_ARGS__)))
#define ACCEL_EVAL4(...) ACCEL_EVAL3(ACCEL_EVAL3(ACCEL_EVAL3(__VA_ARGS__)))
#define ACCEL_EVALN(...) ACCEL_EVAL4(ACCEL_EVAL4(ACCEL_EVAL4(__VA_ARGS__)))

#define ACCEL_MAP_END(...)
#define ACCEL_MAP_OUT

#define ACCEL_MAP_GET_END2(...) 0, ACCEL_MAP_END
#define ACCEL_MAP_GET_END1(...) ACCEL_MAP_GET_END2
#define ACCEL_MAP_GET_ENDN(...) ACCEL_MAP_GET_END1

#define ACCEL_MAP_NEXT0(TEST, NEXT, ...) NEXT ACCEL_MAP_OUT
#define ACCEL_MAP_NEXT1(TEST, NEXT) ACCEL_MAP_NEXT0(TEST, NEXT, 0)
#define ACCEL_MAP_NEXTN(TEST, NEXT) ACCEL_MAP_NEXT1(ACCEL_MAP_GET_ENDN TEST, NEXT)

#define ACCEL_MAP0(F, X, PEEK, ...) F(X) ACCEL_MAP_NEXTN(PEEK, ACCEL_MAP1)(F, PEEK, __VA_ARGS__)
#define ACCEL_MAP1(F, X, PEEK, ...) F(X) ACCEL_MAP_NEXTN(PEEK, ACCEL_MAP0)(F, PEEK, __VA_ARGS__)

/// Maps the supplied macro over the supplied arguments.
#define ACCEL_MAP(F, ...) ACCEL_EVALN(ACCEL_MAP1(F, __VA_ARGS__, ()()(), ()()(), ()()(), 0))

#endif

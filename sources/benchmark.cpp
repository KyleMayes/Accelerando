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

#include <accelerando/benchmark.hpp>

#include <algorithm>
#include <cmath>
#include <numeric>
#include <utility>

namespace accel {

Sample::Sample(uint64_t iterations, Nanoseconds<uint64_t> duration)
    : iterations{iterations}
    , duration{duration}
    , average{duration.count() / static_cast<double>(iterations)} { }

/// An implementation of the Kahan summation algorithm for improved floating point sum accuracy.
struct KahanSummation {
    double sum = 0.0;
    double correction = 0.0;

    KahanSummation() { }

    KahanSummation(double sum, double correction) : sum{sum}, correction{correction} { }

    KahanSummation operator+(double value) const {
        auto y = value - correction;
        auto t = sum + y;
        return {t, (t - sum) - y};
    }

    KahanSummation operator+(Sample sample) const {
        return operator+(sample.average.count());
    }

    template <class T>
    KahanSummation& operator+=(T value) {
        *this = operator+(value);
        return *this;
    }
};

#define RANGE(COLLECTION) COLLECTION.begin(), COLLECTION.end()

double calculate_mean(const std::vector<double>& doubles) {
    return std::accumulate(RANGE(doubles), KahanSummation{}).sum / doubles.size();
}

double calculate_mean(const std::vector<Sample>& samples) {
    return std::accumulate(RANGE(samples), KahanSummation{}).sum / samples.size();
}

double calculate_stddev(const std::vector<Sample>& samples, double mean) {
    std::vector<double> deviations;
    deviations.reserve(samples.size());
    auto f = [=](auto s) { return std::pow(s.average.count() - mean, 2.0); };
    std::transform(RANGE(samples), std::back_inserter(deviations), f);
    return std::sqrt(calculate_mean(deviations));
}

Sample operator+(Sample left, Sample right) {
    return {left.iterations + right.iterations, left.duration + right.duration};
}

LinearRegression::LinearRegression(const std::vector<Sample>& samples) {
    auto sums = std::accumulate(RANGE(samples), Sample{0, Nanoseconds<uint64_t>{0}});
    auto xbar = sums.iterations / static_cast<double>(samples.size());
    auto ybar = sums.duration.count() / static_cast<double>(samples.size());

    // Calculate the y-intercept and the slope.
    KahanSummation numerator, denominator;
    for (auto sample : samples) {
        auto x = sample.iterations - xbar;
        numerator += x * (sample.duration.count() - ybar);
        denominator += std::pow(x, 2.0);
    }
    b1 = Nanoseconds<double>{numerator.sum / denominator.sum};
    b0 = Nanoseconds<double>{ybar - (b1.count() * xbar)};

    // Calculate the goodness of fit.
    KahanSummation ssr, sst;
    for (auto sample : samples) {
        auto f = b0 + (b1 * sample.iterations);
        ssr += std::pow(sample.duration.count() - f.count(), 2.0);
        sst += std::pow(sample.duration.count() - ybar, 2.0);
    }
    r2 = 1.0 - (ssr.sum / sst.sum);
}

BenchmarkReport::BenchmarkReport(std::vector<Sample> samples)
    : samples{std::move(samples)}
    , mean{calculate_mean(this->samples)}
    , stddev{calculate_stddev(this->samples, mean.count())}
    , ols{this->samples} { }

/// A geometric series which produces non-repeating integers.
struct GeometricSeries {
    double value;
    double ratio;

    GeometricSeries(double value, double ratio) : value{value}, ratio{ratio} { }

    uint64_t next() {
        auto previous = static_cast<uint64_t>(value);
        while (true) {
            value *= ratio;
            auto current = static_cast<uint64_t>(value);
            if (current != previous) {
                return current;
            }
        }
    }
};

/// A high-resolution stopwatch.
struct Stopwatch {
    using Clock = std::chrono::high_resolution_clock;

    Clock::time_point start;

    Stopwatch() : start{Clock::now()} { }

    Nanoseconds<uint64_t> get() const {
        return std::chrono::duration_cast<Nanoseconds<uint64_t>>(Clock::now() - start);
    }
};

BenchmarkReport Benchmark::run(Nanoseconds<uint64_t> limit) {
    std::vector<Sample> samples;

    set_up();
    GeometricSeries series{1.0, 1.05};
    Stopwatch stopwatch;
    while (stopwatch.get() < limit) {
        auto iterations = series.next();

        // Collect a sample.
        Stopwatch sample;
        for (uint64_t index = 0; index < iterations; ++index) {
            execute();
        }
        auto duration = sample.get();

        // Discard the sample if it was shorter than 1 millisecond to reduce noise.
        if (duration > Nanoseconds<uint64_t>{1'000'000}) {
            samples.emplace_back(iterations, duration);
        }
    }
    tear_down();

    return {std::move(samples)};
}

}

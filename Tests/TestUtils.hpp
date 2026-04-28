#ifndef TEST_UTILS_HPP_
#define TEST_UTILS_HPP_

#include <cmath>
#include <exception>
#include <functional>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>

namespace test {

class Failure : public std::runtime_error {
public:
    explicit Failure(const std::string& message) : std::runtime_error(message) {}
};

inline void require(bool condition, const std::string& message) {
    if (!condition) {
        throw Failure(message);
    }
}

inline void requireNear(float actual, float expected, float eps, const std::string& label) {
    if (std::fabs(actual - expected) > eps) {
        std::ostringstream oss;
        oss << label << ": expected " << expected << ", got " << actual;
        throw Failure(oss.str());
    }
}

inline void requireThrows(const std::function<void()>& fn, const std::string& label) {
    try {
        fn();
    } catch (const std::exception&) {
        return;
    }
    throw Failure(label + ": expected exception");
}

class Runner {
    int failed = 0;
    int passed = 0;

public:
    void run(const std::string& name, const std::function<void()>& fn) {
        try {
            fn();
            ++passed;
            std::cout << "[PASS] " << name << "\n";
        } catch (const std::exception& ex) {
            ++failed;
            std::cerr << "[FAIL] " << name << ": " << ex.what() << "\n";
        }
    }

    int finish() const {
        std::cout << passed << " passed, " << failed << " failed\n";
        return failed == 0 ? 0 : 1;
    }
};

}  // namespace test

#endif  // TEST_UTILS_HPP_

#pragma once

#include <iostream>
#include <stdexcept>
#include <string>

// ─── Counters ─────────────────────────────────────────────────────────────────

inline int& g_passed() { static int n = 0; return n; }
inline int& g_failed() { static int n = 0; return n; }

// ─── Assertion macros ─────────────────────────────────────────────────────────

#define EXPECT_TRUE(expr)                                                      \
    do {                                                                       \
        if (expr) {                                                            \
            ++g_passed();                                                      \
        } else {                                                               \
            ++g_failed();                                                      \
            std::cerr << "  FAIL  " << __FILE__ << ":" << __LINE__            \
                      << "  " #expr "\n";                                      \
        }                                                                      \
    } while (0)

#define EXPECT_FALSE(expr) EXPECT_TRUE(!(expr))

#define EXPECT_EQ(a, b)                                                        \
    do {                                                                       \
        auto _a = (a);                                                         \
        auto _b = (b);                                                         \
        if (_a == _b) {                                                        \
            ++g_passed();                                                      \
        } else {                                                               \
            ++g_failed();                                                      \
            std::cerr << "  FAIL  " << __FILE__ << ":" << __LINE__            \
                      << "  expected [" << _b << "] got [" << _a << "]\n";    \
        }                                                                      \
    } while (0)

#define EXPECT_NE(a, b)                                                        \
    do {                                                                       \
        if ((a) != (b)) { ++g_passed(); }                                      \
        else {                                                                 \
            ++g_failed();                                                      \
            std::cerr << "  FAIL  " << __FILE__ << ":" << __LINE__            \
                      << "  expected values to differ\n";                      \
        }                                                                      \
    } while (0)

// Passes if expr throws ExcType; fails if it throws something else or nothing.
#define EXPECT_THROW(expr, ExcType)                                            \
    do {                                                                       \
        bool _caught = false;                                                  \
        try { expr; }                                                          \
        catch (const ExcType&) { _caught = true; }                            \
        catch (...) {}                                                         \
        if (_caught) { ++g_passed(); }                                         \
        else {                                                                 \
            ++g_failed();                                                      \
            std::cerr << "  FAIL  " << __FILE__ << ":" << __LINE__            \
                      << "  expected " #ExcType " not thrown\n";               \
        }                                                                      \
    } while (0)

// ─── Suite helper ─────────────────────────────────────────────────────────────

inline void begin_suite(const std::string& name) {
    std::cout << "\n[" << name << "]\n";
}

// ─── Final report ─────────────────────────────────────────────────────────────

inline int report() {
    int total = g_passed() + g_failed();
    std::cout << "\n" << g_passed() << "/" << total << " checks passed";
    if (g_failed() == 0) std::cout << "  ✓\n";
    else                 std::cout << "  " << g_failed() << " FAILED\n";
    return g_failed() == 0 ? 0 : 1;
}

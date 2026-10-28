// test_main.cpp - GoogleTest entry point.
//
// RUN_ALL_TESTS() returns non-zero if any test fails, so the process exit code
// reports failures to the shell and to any build script.
#include "pch.h"

#include <gtest/gtest.h>

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
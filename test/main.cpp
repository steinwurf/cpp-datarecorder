// Copyright (c) 2025 Steinwurf ApS
// All Rights Reserved
//
// Distributed under the "BSD License". See the accompanying LICENSE.rst file.

#include <cstdint>
#include <ctime>

#include <google/protobuf/stubs/common.h>
#include <gtest/gtest.h>

int main(int argc, char** argv)
{
    srand(static_cast<uint32_t>(time(0)));

    testing::InitGoogleTest(&argc, argv);

    // Delete all global objects allocated by libprotobuf. Otherwise Valgrind
    // will complain about memory leaks.
    google::protobuf::ShutdownProtobufLibrary();

    return RUN_ALL_TESTS();
}

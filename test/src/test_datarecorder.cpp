// Copyright (c) 2025 Steinwurf ApS
// All Rights Reserved
//
// Distributed under the "BSD License". See the accompanying LICENSE.rst file.

#include <datarecorder/datarecorder.hpp>
#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>
#include <string>

TEST(datarecorder, record_string)
{
    datarecorder::datarecorder recorder;

    recorder.set_recording_dir("test/recordings");
    std::string data = "hello world";

    auto result = recorder.record(data);
    EXPECT_TRUE(result);

    // Mismatch
    data = "hello world!";
    auto mismatch_result = recorder.record(data);
    EXPECT_FALSE(mismatch_result);
}

TEST(datarecorder, mismatch_directory_only_created_when_needed)
{
    // Test that mismatch directories are not created unnecessarily
    datarecorder::datarecorder recorder;
    recorder.set_recording_dir("test/recordings");

    // Record initial data
    std::string data = "test data for directory creation";
    auto result = recorder.record(data);
    EXPECT_TRUE(result);

    // Count existing mismatch directories before test
    std::filesystem::path tmp_dir = std::filesystem::temp_directory_path();
    int initial_count = 0;
    for (const auto& entry : std::filesystem::directory_iterator(tmp_dir))
    {
        if (entry.is_directory() && 
            entry.path().filename().string().find("cppmismatch-") == 0)
        {
            initial_count++;
        }
    }

    // Record the same data again (no mismatch, no directory should be created)
    result = recorder.record(data);
    EXPECT_TRUE(result);

    // Count directories after test
    int final_count = 0;
    for (const auto& entry : std::filesystem::directory_iterator(tmp_dir))
    {
        if (entry.is_directory() && 
            entry.path().filename().string().find("cppmismatch-") == 0)
        {
            final_count++;
        }
    }

    // No new directories should have been created
    EXPECT_EQ(initial_count, final_count);
}

#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>
#include <string>
#include "datarecorder/data_recorder.hpp"

TEST(DataRecorder, RecordAndCompareData)
{

    util::data_recorder recorder;
    auto tmp_dir = std::filesystem::temp_directory_path() / "cppdatarecorder-test";
    std::filesystem::create_directory(tmp_dir);
    std::string filename = "testfile.data";

    recorder.set_recording_dir(tmp_dir);
    recorder.set_recording_filename(filename);
    std::string data = "hello world";

    // First call should create the file
    auto result = recorder.record(data);
    EXPECT_TRUE(result);

    // Second call should read and compare (no mismatch)
    auto result2 = recorder.record(data);
    EXPECT_TRUE(result2);

    // Clean up
    std::filesystem::remove_all(tmp_dir);
}

TEST(DataRecorder, DetectsMismatchAndCallsHandler)
{
    util::data_recorder recorder;
    auto tmp_dir = std::filesystem::temp_directory_path() / "cppdatarecorder-mismatch";

    std::filesystem::create_directory(tmp_dir);
    std::string filename = "mismatch.data";

    recorder.set_recording_dir(tmp_dir);
    recorder.set_recording_filename(filename);
    std::string data1 = "foo";
    std::string data2 = "bar";

    bool handler_called = false;
    recorder.on_mismatch([&](util::data_mismatch mismatch)
                         {
         handler_called = true;
            EXPECT_EQ(mismatch.recording_data, data1);
        EXPECT_EQ(mismatch.mismatch_data, data2);
        return poke::make_error(std::make_error_code(std::errc::invalid_argument)); });

    EXPECT_TRUE(recorder.record(data1));
    auto result = recorder.record(data2);

    EXPECT_FALSE(result);
    EXPECT_TRUE(handler_called);

    std::filesystem::remove_all(tmp_dir);
}

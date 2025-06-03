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

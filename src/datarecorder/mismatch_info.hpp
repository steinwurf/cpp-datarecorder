// Copyright (c) 2025 Steinwurf ApS
// All Rights Reserved
//
// Distributed under the "BSD License". See the accompanying LICENSE.rst file.

#pragma once

#include <filesystem>
#include <string>

namespace datarecorder
{

/// Mismatch information
struct mismatch_info
{
    /// Data in the recording
    std::string recording_data;

    /// Data that was produced
    std::string mismatch_data;

    /// The path mismatch artifacts can be stored
    std::filesystem::path mismatch_dir;

    /// Recording path (this is where the recording is stored)
    std::filesystem::path recording_path;
};

}
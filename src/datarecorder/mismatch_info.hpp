// Copyright (c) 2025 Steinwurf ApS
// All Rights Reserved
//
// THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF STEINWURF
// The copyright notice above does not evidence any
// actual or intended publication of such source code.

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
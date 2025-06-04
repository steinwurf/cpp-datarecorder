// Copyright (c) 2025 Steinwurf ApS
// All Rights Reserved
//
// Distributed under the "BSD License". See the accompanying LICENSE.rst file.

#pragma once

#include <fmt/format.h>

#include "mismatch_info.hpp"

namespace datarecorder
{

/// Helpers to output the log
inline void to_json_property(fmt::memory_buffer& buffer,
                             const mismatch_info& element)
{
    fmt::format_to(std::back_inserter(buffer), R"("mismatch_dir": "{}")",
                   element.mismatch_dir.string());
}

}
// Copyright (c) 2025 Steinwurf ApS
// All Rights Reserved
//
// THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF STEINWURF
// The copyright notice above does not evidence any
// actual or intended publication of such source code.

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
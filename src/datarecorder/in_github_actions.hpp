// Copyright (c) 2025 Steinwurf ApS
// All Rights Reserved
//
// Distributed under the "BSD License". See the accompanying LICENSE.rst file.

#pragma once

#include <cstdlib>
#include <string_view>

namespace datarecorder
{

inline auto in_github_actions() -> bool
{
    const auto* value = std::getenv("GITHUB_ACTIONS");
    return value != nullptr && std::string_view(value) == "true";
}

} // namespace datarecorder

// Copyright (c) 2025 Steinwurf ApS
// All Rights Reserved
//
// Distributed under the "BSD License". See the accompanying LICENSE.rst file.

#pragma once

#include <gtest/gtest.h>
#include <poke/monitor.hpp>

namespace datarecorder
{

/// This class is used to filter out keys from a JSON object.
/// It removes keys that are unnecessary or irrelevant for the test.
/// Example:
///
///     std::vector<std::string> log_messages;
///
///     auto log = [&](poke::log_level, const std::string_view& message) {
///         log_messages.push_back(
///             filter_json(message).transform_objects(
///                 [](bourne::json& obj) {
///                     if (obj.has_key("pid"))
///                     {
///                         obj["pid"] = 0;
///                     }
///                 })
///         );
///     };
///
///     handler.monitor().enable_log(log, poke::log_level::debug);
struct filter_json
{
    /// Constructor
    filter_json(const std::string_view& data) : filter_json(std::string(data))
    {
    }

    /// Constructor
    filter_json(const std::string& data) : m_json(bourne::json::parse(data))
    {
    }

    /// Constructor
    filter_json(const bourne::json& data) : m_json(data)
    {
    }

    // The bourne::json object should have a single key-value pair
    template <class Visitor>
    auto transform_objects(Visitor visitor) -> filter_json&
    {
        transform_object(m_json, visitor);
        return *this;
    }

    /// Return the filtered JSON object as a string
    auto to_str() const -> std::string
    {
        return to_json().dump_min();
    }

    /// Return the filtered JSON object
    auto to_json() const -> bourne::json
    {
        return m_json;
    }

private:
    template <class Visitor>
    void transform_object(bourne::json& object, Visitor visitor)
    {
        visitor(object);

        for (auto& [key, value] : object.object_range())
        {
            if (value.is_object())
            {
                transform_object(value, visitor);
            }
        }
    }

private:
    /// The JSON object to filter
    bourne::json m_json;
};
}
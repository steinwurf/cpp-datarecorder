// Copyright (c) 2025 Steinwurf ApS
// All Rights Reserved
//
// THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF STEINWURF
// The copyright notice above does not evidence any
// actual or intended publication of such source code.

#pragma once

#include <poke/monitor.hpp>
#include <tl/expected.hpp>
#include <verify/verify.hpp>

#include "data_mismatch.hpp"
#include "to_json_property.hpp"

namespace util
{

/// This class is used to record data and check for mismatches.
///
/// Example:
///     data_recorder recorder;
///     recorder.set_recording_dir("test/recordings/mytest1.json");
///     recorder.on_mismatch([](data_recorder::mismatch mismatch)
///     {
///         std::cout << "Mismatch found!" << std::endl;
///         std::cout << "Recording data: " << mismatch.recording_data
///                   << std::endl;
///         std::cout << "Mismatch data: " << mismatch.mismatch_data
///                   << std::endl;
///         std::cout << "Mismatch path: " << mismatch.mismatch_path
///                   << std::endl;
///     });
///     recorder.record("test data");
///
class data_recorder
{
public:
    /// Constructor
    data_recorder() : m_monitor("data_recorder")
    {
    }

    /// Sets the recording path where data will be stored. The path must include
    /// both the directory and the filename.
    ///
    /// Requirements:
    ///
    /// * If a directory is specified, it must already exist.
    /// * Both absolute and relative paths are supported.
    ///
    /// Path Resolution:
    ///
    /// * If the given path is relative, `data_recorder` will attempt to resolve
    /// it to an absolute path.
    /// * The resolution process searches backward from the current working
    /// directory (cwd), moving up the directory tree until it finds an existing
    /// matching directory.
    ///
    /// Example: Given a cwd of `/home/user/project/build`, calling:
    ///
    ///     data_recorder recorder;
    ///     recorder.set_recording_dir("test/recordings/mytest1.json");
    ///
    /// Will attempt to find the "test/recordings" directory in the following
    /// order:
    ///
    /// - `/home/user/project/build/test/recordings/`
    /// - `/home/user/project/test/recordings/`
    /// - `/home/user/test/recordings/`
    /// - `/home/test/recordings/`
    /// - `/test/recordings/`
    ///
    /// If the directory is found the recording file mytest1.json will be
    /// created in that directory.
    ///
    /// Note, that resolving this may not be the best in all use-cases. So we
    /// may want to revisit this in the future. Additional options like setting
    /// the recording path via an environment variable or as an argument to the
    /// test may be added.
    void set_recording_dir(std::filesystem::path recording_dir)
    {
        VERIFY(!recording_dir.empty(), "Recording path must not be empty",
               recording_dir);

        // Check if the path is absolute
        if (recording_dir.is_absolute())
        {
            m_recording_dir = recording_dir;
            return;
        }

        // The path is relative, so we need to resolve it. First split into
        // directory and filename
        auto base_path = recording_dir.parent_path();
        auto file_name = recording_dir.filename();

        auto cwd = std::filesystem::current_path();

        if (base_path.empty())
        {
            m_recording_dir = cwd / file_name;
            return;
        }

        // Find the recording directory by iterating backwards from the cwd
        // until we find the first directory that exists
        auto find_result = find_relative_path(recording_dir);

        VERIFY(find_result, "Could not find recording path", recording_dir);

        m_recording_dir = *find_result;
    }

    void set_recording_filename(std::string filename)
    {
        // The file extension should be 2 or more characters ".something"
        VERIFY(filename.size() > 2 && filename[0] == '.',
               "Recording filename must start with .something", filename);

        m_recording_filename = filename;
    }

    void on_mismatch(std::function<poke::error(data_mismatch)> callback)
    {
        m_on_mismatch = callback;
    }

    /// This is the base function that will record the data. Other convenience
    /// functions will call this function. But, before they must serialize their
    /// data to a single string.
    auto record(const std::string& data) -> tl::expected<void, poke::error>
    {
        // Check if we have a missmatch handler
        if (!m_on_mismatch)
        {
            determine_mismatch_handler();
        }

        // Check if the recording path is set
        VERIFY(m_recording_dir);

        if (!m_recording_filename)
        {
            m_recording_filename = testname_as_filename();
            m_monitor.log(
                poke::log_level::debug,
                poke::log::str{"message", "Recording filename not set"},
                poke::log::str{"test_name", *m_recording_filename});
        }

        std::filesystem::path recording_path =
            m_recording_dir.value() / m_recording_filename.value();

        // Check if the file exists
        if (std::filesystem::exists(recording_path))
        {
            m_monitor.log(
                poke::log_level::debug,
                poke::log::str{"message", "Recording file already exists"},
                poke::log::str{"path", recording_path.string()});

            // Read the data from the recording path
            std::string recording_data = read_data(recording_path);

            // Compare the data
            return compare_data(data, recording_data);
        }
        else
        {
            m_monitor.log(
                poke::log_level::debug,
                poke::log::str{"message", "Recording file does not exist"},
                poke::log::str{"path", recording_path.string()});

            // If it does not exist we create it
            write_data(recording_path, data);
        }

        // If we get here we are good
        return {};
    }

    auto record(const std::vector<std::string>& data)
        -> tl::expected<void, poke::error>
    {
        // We have to build a single string from the vector
        std::string data_string;
        for (const auto& data : data)
        {
            data_string += data + "\n";
        }
        return record(data_string);
    }

    auto testname_as_filename() -> std::string
    {
        // Get the current test name
        auto* test_info =
            ::testing::UnitTest::GetInstance()->current_test_info();

        std::string test_case = test_info->test_case_name();
        std::string test_name = test_info->name();

        VERIFY(!test_case.empty());
        VERIFY(!test_name.empty());

        std::string filename = test_case + "_" + test_name + ".data";
        return filename;
    }

    auto monitor() -> poke::monitor&
    {
        return m_monitor;
    }

private:
    void determine_mismatch_handler()
    {
        auto visualizer = find_relative_path("visualizer/recording_diff.html");

        if (visualizer)
        {
            m_monitor.log(poke::log_level::debug,
                          poke::log::str{"message", "Using diff visualizer"},
                          poke::log::str{"path", visualizer->string()});

            m_on_mismatch = [this, visualizer](data_mismatch mismatch)
            {
                // Call the diff handler
                return diff_mismatch_handler(*visualizer, mismatch);
            };
        }
        else
        {
            m_monitor.log(
                poke::log_level::debug,
                poke::log::str{"message", "Using default mismatch handler"},
                poke::log::str{"path", visualizer.error().message()});

            m_on_mismatch = [this](data_mismatch mismatch)
            {
                // Call the default handler
                return default_mismatch_handler(mismatch);
            };
        }
    }

    auto determine_mismatch_dir() -> std::filesystem::path
    {
        VERIFY(m_recording_dir, "Recording dir must not be empty");

        // Put the mismatch in /tmp/cppmismatch-N/file_name where N is
        // a concecutive number incremented if already exists
        std::filesystem::path tmp_dir = std::filesystem::temp_directory_path();
        std::filesystem::path mismatch_dir = tmp_dir / "cppmismatch-0";

        std::size_t i = 0;
        while (std::filesystem::exists(mismatch_dir))
        {
            ++i;
            mismatch_dir = tmp_dir / ("cppmismatch-" + std::to_string(i));
        }

        // Create the directory
        std::error_code ec;
        bool created = std::filesystem::create_directory(mismatch_dir, ec);

        VERIFY(created, "Could not create directory", ec);

        return mismatch_dir;
    }

    void write_data(const std::filesystem::path& path, const std::string& data)
    {
        std::ofstream file(path, std::ios::out | std::ios::trunc);
        VERIFY(file.is_open(), "Could not open file for writing", errno, path);

        file << data;
        file.close();

        VERIFY(file.good(), "Could not write to file", errno);
    }

    auto read_data(const std::filesystem::path& path) -> std::string
    {
        std::ifstream file(path, std::ios::in);
        VERIFY(file.is_open(), "Could not open file for reading", errno);

        // Read all data from the file
        std::string data{std::istreambuf_iterator<char>(file),
                         std::istreambuf_iterator<char>()};

        file.close();

        return data;
    }

    auto compare_data(const std::string& data,
                      const std::string& recording_data)
        -> tl::expected<void, poke::error>
    {
        VERIFY(m_recording_filename.has_value(),
               "Recording filename must not be empty");

        // If it exists we check for  a mismatch
        std::filesystem::path mismatch_dir = determine_mismatch_dir();

        if (data != recording_data)
        {
            m_monitor.log(poke::log_level::debug,
                          poke::log::str{"message", "Mismatch found"});

            // We have a mismatch
            data_mismatch mismatch;
            mismatch.recording_data = recording_data;
            mismatch.mismatch_data = data;
            mismatch.mismatch_dir = mismatch_dir;

            VERIFY(m_recording_filename.has_value());
            VERIFY(m_recording_dir.has_value());

            mismatch.recording_path =
                m_recording_dir.value() / m_recording_filename.value();

            VERIFY(m_on_mismatch, "Mismatch handler not set");
            return tl::make_unexpected(m_on_mismatch.value()(mismatch));
        }
        else
        {
            m_monitor.log(poke::log_level::debug,
                          poke::log::str{"message", "No mismatch found"});

            return {};
        }
    }

    auto find_relative_path(const std::filesystem::path& path) const
        -> tl::expected<std::filesystem::path, poke::error>
    {
        // We'll store where we looked for the path - just for
        // debugging purposes
        std::vector<std::filesystem::path> searched_paths;

        // Iterate backwards from the current working directory until we
        // find the first directory that exists
        auto current_path = std::filesystem::current_path();
        while (!current_path.empty())
        {
            searched_paths.push_back(current_path / path);
            if (std::filesystem::exists(current_path / path))
            {
                return current_path / path;
            }

            current_path = current_path.parent_path();
        }

        // If we get here, we could not find the path
        std::string searched_paths_str;
        for (const auto& path : searched_paths)
        {
            searched_paths_str += path.string() + "\n";
        }

        return tl::make_unexpected(poke::make_error(
            std::make_error_code(std::errc::no_such_file_or_directory),
            poke::log::str{"searched_paths", searched_paths_str},
            poke::log::str{"path", path.string()}));
    }

    auto diff_mismatch_handler(std::filesystem::path recording_diff_html,
                               data_mismatch mismatch) -> poke::error
    {

        m_monitor.log(
            poke::log_level::debug,
            poke::log::str{"message", "Using diff mismatch handler"},
            poke::log::str{"recording_diff_html", recording_diff_html.string()},
            mismatch);

        auto escape_dollar_bracs = [](const std::string& input)
        {
            // When we insert the string in the HTML file we need to escape
            // the dollar brackets. Since this is interpreted as a template
            // string litteral in javascript. So we need to escape it with a
            // backslash.
            static const std::regex pattern(R"(\$\{[^}]+\})");
            return std::regex_replace(input, pattern, R"(\$&)");
        };

        std::string escaped_recording_data =
            escape_dollar_bracs(mismatch.recording_data);
        std::string escaped_mismatch_data =
            escape_dollar_bracs(mismatch.mismatch_data);

        std::string file_content = read_data(recording_diff_html);

        std::regex oldTextPattern(R"((const\s+oldText\s*=\s*`)([^`]*)(`;))");
        std::regex newTextPattern(R"((const\s+newText\s*=\s*`)([^`]*)(`;))");

        file_content = std::regex_replace(file_content, oldTextPattern,
                                          "$1" + escaped_recording_data + "$3");
        file_content = std::regex_replace(file_content, newTextPattern,
                                          "$1" + escaped_mismatch_data + "$3");

        // Output file
        std::filesystem::path output_file =
            mismatch.mismatch_dir / recording_diff_html.filename();

        // Write the modified content back to the file
        write_data(output_file, file_content);

        // Also write the mismatch data to the mismatch dir
        std::filesystem::path mismatch_path =
            mismatch.mismatch_dir / mismatch.recording_path.filename();

        write_data(mismatch_path, mismatch.mismatch_data);

        return poke::make_error(
            std::make_error_code(std::errc::invalid_argument),
            poke::log::str{"message", "Mismatch found"},
            poke::log::str{"recording_data:", mismatch.recording_data},
            poke::log::str{"mismatch_data:", mismatch.mismatch_data},
            poke::log::str{"recording_path:", mismatch.recording_path.string()},
            poke::log::str{"mismatch_path:", mismatch_path.string()},
            poke::log::str{"html_diff", output_file.string()});
    }

    auto default_mismatch_handler(data_mismatch mismatch) -> poke::error

    {
        /// We just return the mismatch as strings
        return poke::make_error(
            std::make_error_code(std::errc::invalid_argument),
            poke::log::str{"recording_data:", mismatch.recording_data},
            poke::log::str{"mismatch_data:", mismatch.mismatch_data});
    }

private:
    /// Monitor for logging
    poke::monitor m_monitor;

    std::optional<std::string> m_recording_filename;
    std::optional<std::filesystem::path> m_recording_dir;
    std::optional<std::function<poke::error(data_mismatch)>> m_on_mismatch;
};

}
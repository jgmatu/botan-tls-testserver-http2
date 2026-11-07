#pragma once

/**
 * @file
 * @brief Command line parsing.
 * @ingroup server
 */

#include "server/ServerOptions.hpp"

#include <string>

/// @brief Outcome of parse_command_line().
struct CommandLineResult {
      /// @brief What main() should do next.
      enum class Status {
         Run,    ///< options are valid, start the server
         Help,   ///< usage was requested (or no arguments given); message holds the text
         Error,  ///< invalid arguments; message holds the reason
      };

      Status status = Status::Run;  ///< What to do next.
      ServerOptions options;        ///< Parsed options; only meaningful for Status::Run.
      std::string message;          ///< Usage text or error reason.
};

/**
 * @brief Parses the command line with Boost.Program_options.
 *
 * --port, --cert and --key are required. Run the binary with --help for the
 * full list and defaults.
 *
 * @param argc Argument count from main().
 * @param argv Arguments from main().
 * @return The parsed options or the text to print. Never throws.
 */
CommandLineResult parse_command_line(int argc, const char* const argv[]);

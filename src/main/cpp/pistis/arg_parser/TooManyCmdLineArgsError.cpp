#include "TooManyCmdLineArgsError.hpp"

using namespace pistis::arg_parser;

TooManyCmdLineArgsError::TooManyCmdLineArgsError(const std::string& appName):
    CmdLineArgError(appName, "Too many command-line arguments") {
  // Intentionally left blank
}


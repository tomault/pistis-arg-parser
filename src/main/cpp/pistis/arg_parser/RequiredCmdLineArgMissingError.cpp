#include "RequiredCmdLineArgMissingError.hpp"

using namespace pistis::arg_parser;

RequiredCmdLineArgMissingError::RequiredCmdLineArgMissingError(
    const std::string& appName, const std::string& argName
):
    CmdLineArgError(appName, argName + " not specified.  Use -h for help.") {
}

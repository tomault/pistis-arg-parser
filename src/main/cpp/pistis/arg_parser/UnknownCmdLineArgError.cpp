#include "UnknownCmdLineArgError.hpp"
#include <sstream>

using namespace pistis::arg_parser;

UnknownCmdLineArgError::UnknownCmdLineArgError(const std::string& appName,
					       const std::string& argName):
    CmdLineArgError(appName, _createMessage(argName)) {
  // Intentionally left blank
}

std::string UnknownCmdLineArgError::_createMessage(
    const std::string& argName
) {
  std::ostringstream msg;
  msg << "Unknown command-line argument";
  if (!argName.empty()) {
    msg << " " << argName;
  }
  return msg.str();
}

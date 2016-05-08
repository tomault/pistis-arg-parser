#include "CmdLineArgError.hpp"
#include <sstream>

using namespace pistis::exceptions;
using namespace pistis::arg_parser;

CmdLineArgError::CmdLineArgError(const std::string& appName,
				 const std::string& details):
  PistisException(createMessage_(appName, details)) {
}

std::string CmdLineArgError::createMessage_(const std::string& appName,
					    const std::string& details) {
  std::ostringstream msg;
  if (!appName.empty()) {
    msg << appName << ": ";
  }
  if (!details.empty()) {
    msg << details;
  } else {
    msg << "Error parsing command-line arguments";
  }
  return msg.str();
}

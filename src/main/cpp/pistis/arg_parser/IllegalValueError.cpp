#include "IllegalValueError.hpp"
#include <sstream>

using namespace pistis::arg_parser;

IllegalValueError::IllegalValueError(const std::string& appName,
				     const std::string& argName,
				     const char* value):
  CmdLineArgError(appName, createMessage_(argName, value, "")) {
}

IllegalValueError::IllegalValueError(const std::string& appName,
				     const std::string& argName,
				     const char* value,
				     const std::string& details):
  CmdLineArgError(appName, createMessage_(argName, value, details)) {
}

std::string IllegalValueError::createMessage_(const std::string& argName,
					      const char* value,
					      const std::string& details) {
  std::ostringstream msg;
  
  msg << "Illegal value";
  if (value && value[0]) {
    msg << " \"" << value << "\"";
  }
  if (!argName.empty()) {
    msg << " for command-line argument " << argName;
  } else {
    msg << " on the command-line";
  }
  if (!details.empty()) {
    msg << " (" << details << ")";
  }
  return msg.str();
}

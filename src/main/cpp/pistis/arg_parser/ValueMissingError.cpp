#include "ValueMissingError.hpp"
#include <sstream>

using namespace pistis::arg_parser;

ValueMissingError::ValueMissingError(const std::string& appName,
				     const std::string& argName):
    CmdLineArgError(appName, createMessage_(argName)) {
  // Intentionally left blank
}
	
std::string ValueMissingError::createMessage_(const std::string& argName) {
  if (!argName.empty()) {
    std::ostringstream msg;
    msg << "Value missing for " << argName;
    return msg.str();
  } else {
    return std::string("Required value missing on the command-line");
  }
}

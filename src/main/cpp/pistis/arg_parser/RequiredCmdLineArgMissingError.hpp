#ifndef __PISTIS__ARG_PARSER__REQUIREDCMDLINEARGMISSINGERROR_HPP__
#define __PISTIS__ARG_PARSER__REQUIREDCMDLINEARGMISSINGERROR_HPP__

#include <pistis/arg_parser/CmdLineArgError.hpp>

namespace pistis {
  namespace arg_parser {

      class RequiredCmdLineArgMissingError : public CmdLineArgError {
      public:
	RequiredCmdLineArgMissingError(const std::string& appName,
				       const std::string& argName);
      };
    
  }
}
#endif


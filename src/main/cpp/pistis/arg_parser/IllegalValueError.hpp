#ifndef __PISTIS__ARG_PARSER__ILLEGALVALUEERROR_HPP__
#define __PISTIS__ARG_PARSER__ILLEGALVALUEERROR_HPP__

#include <pistis/arg_parser/CmdLineArgError.hpp>
#include <string>

namespace pistis {
  namespace arg_parser {

      class IllegalValueError : public CmdLineArgError {
      public:
	IllegalValueError(const std::string& appName,
			  const std::string& argName,
			  const char* value);
	IllegalValueError(const std::string& appName,
			  const std::string& argName,
			  const char* value,
			  const std::string& details);

      private:
	static std::string createMessage_(const std::string& argName,
					  const char* value,
					  const std::string& details);
      };

  }
}
#endif


#ifndef __PISTIS__ARG_PARSER__VALUEMISSINGERROR_HPP__
#define __PISTIS__ARG_PARSER__VALUEMISSINGERROR_HPP__

#include <pistis/arg_parser/CmdLineArgError.hpp>
#include <string>

namespace pistis {
  namespace arg_parser {

    class ValueMissingError : public CmdLineArgError {
    public:
      ValueMissingError(const std::string& appName,
			const std::string& argName);

    private:
      static std::string createMessage_(const std::string& argName);
    };

  }
}

#endif


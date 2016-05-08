#ifndef __PISTIS__UTIL__ARGS__UNKNOWNCMDLINEARGERROR_HPP__
#define __PISTIS__UTIL__ARGS__UNKNOWNCMDLINEARGERROR_HPP__

#include <pistis/arg_parser/CmdLineArgError.hpp>

namespace pistis {
  namespace arg_parser {

    class UnknownCmdLineArgError : public CmdLineArgError {
    public:
      UnknownCmdLineArgError(const std::string& appName,
			     const std::string& argName);

    private:
      static std::string _createMessage(const std::string& argName);
    };

  }
}
#endif


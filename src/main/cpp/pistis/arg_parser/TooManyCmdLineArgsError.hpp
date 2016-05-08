#ifndef __PISTIS__ARG_PARSER__TOOMANYCMDLINEARGSERROR_HPP__
#define __PISTIS__ARG_PARSER__TOOMANYCMDLINEARGSERROR_HPP__

#include <pistis/arg_parser/CmdLineArgError.hpp>

namespace pistis {
  namespace arg_parser {

    class TooManyCmdLineArgsError : public CmdLineArgError {
    public:
      TooManyCmdLineArgsError(const std::string& appName);
    };

  }
}
#endif


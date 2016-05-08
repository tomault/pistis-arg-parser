#ifndef __PISTIS__ARG_PARSER__CMDLINEARGERROR_HPP__
#define __PISTIS__ARG_PARSER__CMDLINEARGERROR_HPP__

#include <pistis/exceptions/PistisException.hpp>

namespace pistis {
  namespace arg_parser {

    class CmdLineArgError : public exceptions::PistisException {
    public:
      CmdLineArgError(const std::string& appName, const std::string& details);

    private:
      static std::string createMessage_(const std::string& appName,
					const std::string& details);
    };

  }
}

#endif

#ifndef __PISTIS__ARG_PARSER__ABSTRACTCMDLINEARGS_HPP__
#define __PISTIS__ARG_PARSER__ABSTRACTCMDLINEARGS_HPP__

#include <string>

namespace pistis {
  namespace arg_parser {

    class CmdLineArgGenerator;

    class AbstractCmdLineArgs {
    public:
      AbstractCmdLineArgs();
      virtual ~AbstractCmdLineArgs() { }

      void parse(int argc, char** argv);
      bool showUsage() const { return showUsage_; }

    protected:
      virtual void init_(int argc, char** argv);
      virtual bool handleNamedArg_(CmdLineArgGenerator& args,
				   const std::string& argName);
      virtual bool handleUnnamedArg_(CmdLineArgGenerator& args,
				     const std::string& value);
      virtual void check_(const std::string& appName);

    private:
      bool showUsage_;
    };

  }
}
#endif

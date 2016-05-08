#include "AbstractCmdLineArgs.hpp"
#include "CmdLineArgGenerator.hpp"
#include "TooManyCmdLineArgsError.hpp"
#include "UnknownCmdLineArgError.hpp"

using namespace pistis::arg_parser;

AbstractCmdLineArgs::AbstractCmdLineArgs():
  showUsage_(false) {
}

void AbstractCmdLineArgs::parse(int argc, char **argv) {
  init_(argc, argv);
  CmdLineArgGenerator args(argc, argv);
  while (args.remaining()) {
    std::string arg= args.next();
    if (!arg.empty() && (arg[0] == '-')) {
      if (!handleNamedArg_(args, arg)) {
	throw UnknownCmdLineArgError(args.appName(), arg);
      }
    } else {
      if (!handleUnnamedArg_(args, arg)) {
	throw TooManyCmdLineArgsError(args.appName());
      }
    }
  }
  check_(args.appName());
}

void AbstractCmdLineArgs::init_(int argc, char **argv) {
  showUsage_ = false;
}

bool AbstractCmdLineArgs::handleNamedArg_(CmdLineArgGenerator& args,
					  const std::string& argName) {
  if ((argName == "-h") || (argName == "--help")) {
    showUsage_ = true;
    return true;
  }
  return false;
}

bool AbstractCmdLineArgs::handleUnnamedArg_(CmdLineArgGenerator& args,
					    const std::string& value) {
  return false;
}

void AbstractCmdLineArgs::check_(const std::string& appName) {
  // Default implementation does nothing
}

#include "SimpleCmdLineArgs.hpp"
#include "RequiredCmdLineArgMissingError.hpp"
#include <pistis/exceptions/IllegalStateError.hpp>
#include <pistis/exceptions/IllegalValueError.hpp>

using pistis::exceptions::PistisException;
using namespace pistis::arg_parser;

SimpleCmdLineArgs::SimpleCmdLineArgs():
    AbstractCmdLineArgs(), namedArgs_(), unnamedArgs_(), currentUnnamedArg_() {
}

SimpleCmdLineArgs::~SimpleCmdLineArgs() {
  for (auto i= namedArgs_.begin(); i != namedArgs_.end(); ++i) {
    delete i->second;
  }
  for (auto i= unnamedArgs_.begin(); i != unnamedArgs_.end(); ++i) {
    delete *i;
  }
}

void SimpleCmdLineArgs::registerNamedArg_(const std::string& argName,
					  const std::string& description,
					  bool required,
					  const std::function<
					      void (CmdLineArgGenerator&,
						    const std::string&)
					  >& handler) {
  ArgHandler* h=
    createDelegate_(argName, description, required, true,
		    [handler](CmdLineArgGenerator& args,
			      const std::string& argName) {
      try {
	handler(args, argName);
      } catch(const FormatError& e) {
	throw;
      } catch(const CmdLineArgError& e) {
	throw;
      } catch(const std::exception& e) {
	throw FormatError(e.what());
      } catch(...) {
	throw FormatError(std::string());
      }
    });
  registerHandler_(h);
}

void SimpleCmdLineArgs::registerUnnamedArg_(const std::string& description,
					    bool required,
					    const std::function<
					        void (CmdLineArgGenerator&,
						      const std::string&)
					    >& handler) {
  ArgHandler* h=
    createDelegate_(std::string(), description, required, false,
		    [handler](CmdLineArgGenerator& args,
			      const std::string& argValue) {
      try {
	handler(args, argValue);
      } catch(const FormatError& e) {
	throw;
      } catch(const CmdLineArgError& e) {
	throw;
      } catch(const std::exception& e) {
	throw FormatError(e.what());
      } catch(...) {
	throw FormatError(std::string());
      }
    });
  registerHandler_(h);
}

void SimpleCmdLineArgs::registerHandler_(ArgHandler* handler) {
  std::unique_ptr<ArgHandler> h(handler);
  if (h->argName().empty()) {
    if (!unnamedArgs_.empty() && unnamedArgs_.back()->final()) {
      throw pistis::exceptions::IllegalStateError(
          "Previous handler for unnamed arguments can accept any number of "
	  "unnamed arguments, so no further handlers for unnamed arguments "
	  "are allowed", PISTIS_EX_HERE
      );
    }
    unnamedArgs_.push_back(h.release());
  } else if (h->argName()[0] != '-') {
    throw pistis::exceptions::IllegalValueError(
        "handler->argName()",  h->argName(),
	"Named arguments must begin with a '-'", PISTIS_EX_HERE
    );
  } else if (namedArgs_.find(h->argName()) != namedArgs_.end()) {
    throw pistis::exceptions::IllegalStateError(
        "Argument \"" + h->argName() +
	"\" already has a handler registered for it",
	PISTIS_EX_HERE
    );
  } else {
    namedArgs_.insert(std::make_pair(h->argName(), h.get()));
    h.release();
  }
}

void SimpleCmdLineArgs::init_(int argc, char** argv) {
  AbstractCmdLineArgs::init_(argc, argv);
  for (auto i= namedArgs_.begin(); i != namedArgs_.end(); ++i) {
    i->second->setFound(false);
  }
  for (auto i= unnamedArgs_.begin(); i != unnamedArgs_.end(); ++i) {
    (*i)->setFound(false);
  }
  currentUnnamedArg_= unnamedArgs_.begin();
  initValues_();
}

bool SimpleCmdLineArgs::handleNamedArg_(CmdLineArgGenerator& args,
					const std::string& argName) {
  if (AbstractCmdLineArgs::handleNamedArg_(args, argName)) {
    return true;
  } else {
    HandlerMapType::iterator i= namedArgs_.find(argName);
    if (i != namedArgs_.end()) {
      try {
	i->second->handleValue(args, argName);
	i->second->setFound(true);
	return true;
      } catch(const FormatError& e) {
	throw IllegalValueError(args.appName(), i->second->fullName(),
				e.value().c_str(), e.details());
      } catch(const CmdLineArgError& e) {
	throw;
      } catch(const std::exception& e) {
	throw IllegalValueError(args.appName(), i->second->fullName(), "",
				e.what());
      } catch(...) {
	throw IllegalValueError(args.appName(), i->second->fullName(), "");
      }
    }
    return false;
  }
}

bool SimpleCmdLineArgs::handleUnnamedArg_(CmdLineArgGenerator& args,
					  const std::string& argValue) {
  if (AbstractCmdLineArgs::handleUnnamedArg_(args, argValue)) {
    return true;
  } else if (currentUnnamedArg_ == unnamedArgs_.end()) {
    return false;
  } else {
    ArgHandler* h= *currentUnnamedArg_;
    try {
      h->handleValue(args, argValue);
      h->setFound(true);
      if (!h->final()) {
	++currentUnnamedArg_;
      }
      return true;
    } catch(const FormatError& e) {
      throw IllegalValueError(args.appName(), h->fullName(), e.value().c_str(),
			      e.details());
    } catch(const CmdLineArgError& e) {
      throw;
    } catch(const std::exception& e) {
      throw IllegalValueError(args.appName(), h->fullName(), "", e.what());
    } catch(...) {
      throw IllegalValueError(args.appName(), h->fullName(), "");
    }
  }
}

void SimpleCmdLineArgs::check_(const std::string& appName) {
  AbstractCmdLineArgs::check_(appName);
  for (auto i= namedArgs_.begin(); i != namedArgs_.end(); ++i) {
    if (i->second->required() && !i->second->found()) {

      throw RequiredCmdLineArgMissingError(appName, i->second->fullName());
    }
  }
  for (auto i= currentUnnamedArg_; i != unnamedArgs_.end(); ++i) {
    ArgHandler* h= *i;
    if (h->required() && !h->found()) {
      throw RequiredCmdLineArgMissingError(appName, h->fullName());
    }
  }
  checkValues_();
}

void SimpleCmdLineArgs::initValues_() {
  // Default implementation does nothing
}

void SimpleCmdLineArgs::checkValues_() {
  // Default implementation does nothing
}

SimpleCmdLineArgs::FormatError::FormatError(const std::string& details):
    PistisException(createMessage_("", details)), value_(), details_(details) {
  // Intentionally left blank
}

SimpleCmdLineArgs::FormatError::FormatError(const std::string& value,
					    const std::string& details):
    PistisException(createMessage_(value, details)), value_(value),
		    details_(details) {
  // Intentionally left blank
}

std::string SimpleCmdLineArgs::FormatError::createMessage_(
    const std::string& value, const std::string& details
) {
  std::ostringstream msg;
  msg << "Formatting error";
  if (!value.empty()) {
    msg << " for \"" << value << "\"";
  }
  if (!details.empty()) {
    msg << " (" << details << ")";
  }
  return msg.str();
}

SimpleCmdLineArgs::ArgHandler::ArgHandler(const std::string& argName,
					  const std::string& description,
					  bool isRequired,
					  bool isFinal):
    argName_(argName), description_(description), required_(isRequired),
    final_(isFinal), found_(false) {
  // Intentionally left blank
}

std::string SimpleCmdLineArgs::ArgHandler::fullName() const {
  if (!argName_.empty() && !description_.empty()) {
    std::ostringstream tmp;
    tmp << description_ << " (" << argName_ << ")";
    return tmp.str();
  } else if (!description_.empty()) {
    return description_;
  } else {
    return argName_;
  }
}


#include "CmdLineArgGenerator.hpp"
#include "ValueMissingError.hpp"
#include <pistis/exceptions/IllegalValueError.hpp>
#include <pistis/util/NumUtil.hpp>

namespace util = pistis::util;
using namespace pistis::arg_parser;

CmdLineArgGenerator::CmdLineArgGenerator(int argc, char** argv):
    args_(argv+1), end_(argv+argc), current_(args_),
    appName_((argc > 0) ? std::string(argv[0]) : std::string()) {
  if (argc < 1) {
    throw pistis::exceptions::IllegalValueError("argc", "must be > 0",
						PISTIS_EX_HERE);
  }
}

std::string CmdLineArgGenerator::current(const std::string& argName) const {
  if (current_ == end_) {
    throw ValueMissingError(appName(), argName);
  }
  return std::string(*current_);
}

std::string CmdLineArgGenerator::next(const std::string& argName) {
  if (current_ == end_) {
    throw ValueMissingError(appName(), argName);
  }
  return std::string(*current_++);
}

int64_t CmdLineArgGenerator::nextAsInt(const std::string& argName) {
  try {
    return nextAs(argName, [](const std::string& argName,
			      const std::string& v) {
      return util::toInt64(v, 10);
    });
  } catch(const pistis::exceptions::IllegalValueError& e) {
    throw IllegalValueError(appName(), argName, *current_,
			    "Must be an integer");
  }
}
									      
int64_t CmdLineArgGenerator::nextAsIntInRange(const std::string& argName,
					      int64_t minValue,
					      int64_t maxValue) {
  int64_t v= nextAsInt(argName);
  if ((v < minValue) || (v > maxValue)) {
    std::ostringstream msg;
    msg << "Must be an integer";
    if ((minValue != INT64_MIN) && (maxValue != INT64_MAX)) {
      msg << " between " << minValue << " and " << maxValue;
    } else if (minValue != INT64_MIN) {
      msg << " greater than or equal to " << minValue;
    } else {
      msg << " less than or equal to " << maxValue;
    }
    --current_;  // Put back the invalid argument
    throw IllegalValueError(appName(), argName, *current_, msg.str());
  }
  return v;
}

uint64_t CmdLineArgGenerator::nextAsUInt(const std::string& argName) {
  try {
    return nextAs(argName, [](const std::string& argName,
			      const std::string& value) {
      return util::toUInt64(value, 10);
    });
  } catch(const pistis::exceptions::IllegalValueError& e) {
    throw IllegalValueError(appName(), argName, *current_,
			    "Must be an integer > 0");
  }
}

uint64_t CmdLineArgGenerator::nextAsUIntInRange(const std::string& argName,
						uint64_t minValue,
						uint64_t maxValue) {
  uint64_t v= nextAsUInt(argName);
  if ((v < minValue) || (v > maxValue)) {
    std::ostringstream msg;
    msg << "Must be an integer";
    if (maxValue != UINT64_MAX) {
      msg << " between " << minValue << " and " << maxValue;
    } else {
      msg << " greater than or equal to " << minValue;
    }
    --current_;
    throw IllegalValueError(appName(), argName, *current_, msg.str());
  }
}

namespace {
  struct ConvertDouble {
    double operator()(const std::string& argName, const std::string& v) const {
      return util::toDouble(v);
    }
  };
}

double CmdLineArgGenerator::nextAsDouble(const std::string& argName) {
  try {
    return nextAs(argName, [](const std::string& name,
			      const std::string& value) {
       return util::toDouble(value);
    });
  } catch(const pistis::exceptions::IllegalValueError& e) {
    throw IllegalValueError(appName(), argName, *current_,
			    "Must be a floating-point number");
  }
}

double CmdLineArgGenerator::nextAsDoubleInRange(const std::string& argName,
						double minValue,
						double maxValue) {
  double v= nextAsDouble(argName);
  if ((v < minValue) || (v > maxValue)) {
    std::ostringstream msg;
    msg << "Must be a floating-point number";
    if ((minValue != DBL_MIN) && (maxValue != DBL_MAX)) {
      msg << " between " << minValue << " and " << maxValue
	  << " inclusive";
    } else if (minValue != DBL_MIN) {
      msg << " greater than or equal to " << minValue;
    } else {
      msg << " less than or equal to " << maxValue;
    }
    --current_;
    throw IllegalValueError(appName(), argName, *current_, msg.str());
  }
  return v;
}


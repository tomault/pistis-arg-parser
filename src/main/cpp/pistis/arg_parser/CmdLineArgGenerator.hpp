#ifndef __PISTIS__ARG_PARSER__CMDLINEARGGENERATOR_HPP__
#define __PISTIS__ARG_PARSER__CMDLINEARGGENERATOR_HPP__

#include <pistis/util/StringUtil.hpp>
#include <pistis/arg_parser/IllegalValueError.hpp>
#include <string>
#include <unordered_set>
#include <float.h>
#include <limits.h>
#include <stdint.h>

namespace pistis {
  namespace arg_parser {

    class CmdLineArgGenerator {
    public:
      CmdLineArgGenerator(int argc, char** argv);

      const std::string appName() const { return appName_; }
      size_t numArgs() const { return (size_t)(end_ - args_); }
      size_t remaining() const { return (size_t)(end_ - current_); }
      
      std::string current(const std::string& argName = std::string()) const;
      std::string next(const std::string& arg = std::string());

      template <typename Converter>
      double foo(const std::string& argName, Converter convert) {
	return convert(argName, next(argName));
      }

      template <typename Converter>
      auto nextAs(const std::string& argName, Converter convert) {
	try {
	  return convert(argName, next(argName));
	} catch(...) {
	  --current_; // Put back the bad argument
	  throw;
	}
      }

      template <typename Set>
      std::string nextInSet(const std::string& argName,
			    const Set& legalValues) {
	std::string v= next(argName);
	if (legalValues.find(v) == legalValues.end()) {
	  std::ostringstream msg;
	  msg << "Must be one of \""
	      << util::join(legalValues.begin(), legalValues.end(), "\", \"")
	      << "\"";
	  --current_;
	  throw IllegalValueError(appName(), argName, v.c_str(), msg.str());
	}
	return v;
      }

      template <typename SetT>
      std::string nextInSet(const SetT& legalValues) {
	return nextInSet(std::string(), legalValues);
      }

      std::string nextInSet(
	  const std::string& argName,
	  const std::initializer_list<std::string>& legalValues
      ) {
	return nextInSet(argName,
			 std::unordered_set<std::string>(legalValues));
      }

      std::string nextInSet(
	  const std::initializer_list<std::string>& legalValues
      ) {
	return nextInSet(std::string(),
			 std::unordered_set<std::string>(legalValues));
      }

      int64_t nextAsInt(const std::string& argName = std::string());
      int64_t nextAsIntInRange(const std::string& argName,
			       int64_t minValue, int64_t maxValue = INT64_MAX);
      int64_t nextAsIntInRange(int64_t minValue,
			       int64_t maxValue = INT64_MAX) {
	return nextAsIntInRange(std::string(), minValue, maxValue);
      }

      uint64_t nextAsUInt(const std::string& argName = std::string());
      uint64_t nextAsUIntInRange(const std::string& argName,
				 uint64_t minValue,
				 uint64_t maxValue = UINT64_MAX);
      uint64_t nextAsUIntInRange(uint64_t minValue,
				 uint64_t maxValue = UINT64_MAX) {
	return nextAsUIntInRange(std::string(), minValue, maxValue);
      }

      double nextAsDouble(const std::string& argName = std::string());
      double nextAsDoubleInRange(const std::string& argName,
				 double minValue, double maxValue = DBL_MAX);
      double nextAsDoubleInRange(double minValue, double maxValue = DBL_MAX) {
	nextAsDoubleInRange(std::string(), minValue, maxValue);
      }

    private:
      char **args_;
      char **end_;
      char **current_;
      const std::string appName_;
    };
    
  }
}

#endif

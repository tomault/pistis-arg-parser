/** @file SimpleCmdLineArgsTest.hpp
 *
 *  Unit tests for pistis::arg_parser::SimpleCmdLineArgs.
 */

#include <pistis/arg_parser/SimpleCmdLineArgs.hpp>
#include <pistis/arg_parser/RequiredCmdLineArgMissingError.hpp>
#include <pistis/arg_parser/IllegalValueError.hpp>
#include <pistis/arg_parser/TooManyCmdLineArgsError.hpp>
#include <pistis/arg_parser/UnknownCmdLineArgError.hpp>
#include <pistis/arg_parser/ValueMissingError.hpp>
#include <pistis/util/StringUtil.hpp>
#include <gtest/gtest.h>
#include <algorithm>
#include <ostream>
#include <unordered_set>

using namespace pistis::arg_parser;
namespace util = pistis::util;

namespace {
  enum class TestEnum {
    NONE, ONE, TWO, THREE
  };
}

namespace std {
  template<>
  struct hash<TestEnum> {
    uint32_t operator()(TestEnum x) const { return (uint32_t)x; }
  };
}

namespace {

  inline std::ostream& operator<<(std::ostream& out, TestEnum e) {
    switch (e) {
      case TestEnum::NONE:  return out << "NONE";
      case TestEnum::ONE:   return out << "ONE";
      case TestEnum::TWO:   return out << "TWO";
      case TestEnum::THREE: return out << "THREE";
      default: return out << "**UNKNOWN**";
    }
  }

  class AnySimpleCmdLineArgs : public SimpleCmdLineArgs {
  public:
    AnySimpleCmdLineArgs():
      SimpleCmdLineArgs(), initValuesCalled_(false), checkValuesCalled_(false) {
    }

    virtual void reset() {
      initValuesCalled_= false;
      checkValuesCalled_= false;
    }
    bool initValuesCalled() const { return initValuesCalled_; }
    bool checkValuesCalled() const { return checkValuesCalled_; }

  protected:
    virtual void initValues_() { initValuesCalled_= true; }
    virtual void checkValues_() { checkValuesCalled_= true; }

    static ValueMap<TestEnum> testEnumValueMap_() {
      ValueMap<TestEnum> valueMap;
      valueMap.setValue("one", TestEnum::ONE);
      valueMap.setValue("two", TestEnum::TWO);
      valueMap.setValue("three", TestEnum::THREE);
      return valueMap;
    }

  private:
    bool initValuesCalled_;
    bool checkValuesCalled_;
  };

  class SingleValueCmdLineArgs : public AnySimpleCmdLineArgs {
  public:
    SingleValueCmdLineArgs();

    int intValue() const { return i_; }
    double doubleValue() const { return d_; }
    const std::string& strValue() const { return s_; }
    TestEnum enumValue() const { return e_; }
    const std::string& formattedValue() { return f_; }
  
    virtual void reset() {
      AnySimpleCmdLineArgs::reset();
      i_= 0;
      d_= 0.0;
      s_.clear();
      e_= TestEnum::NONE;
      f_.clear();
    }
    
  protected:
    int i_;
    double d_;
    std::string s_;
    TestEnum e_;
    std::string f_;
  };

  class NamedSingleValueCmdLineArgs : public SingleValueCmdLineArgs {
  public:
    NamedSingleValueCmdLineArgs();
  };

  class NamedSingleValueInRangeCmdLineArgs : public SingleValueCmdLineArgs {
  public:
    NamedSingleValueInRangeCmdLineArgs();
  };

  class NamedSingleValueInSetCmdLineArgs : public SingleValueCmdLineArgs {
  public:
    NamedSingleValueInSetCmdLineArgs();
  };

  class UnnamedSingleValueCmdLineArgs : public SingleValueCmdLineArgs {
  public:
    UnnamedSingleValueCmdLineArgs();
  };

  class UnnamedSingleValueInRangeCmdLineArgs : public SingleValueCmdLineArgs {
  public:
    UnnamedSingleValueInRangeCmdLineArgs();
  };

  class UnnamedSingleValueInSetCmdLineArgs : public SingleValueCmdLineArgs {
  public:
    UnnamedSingleValueInSetCmdLineArgs();
  };

  class SetCmdLineArgs : public AnySimpleCmdLineArgs {
  public:
    SetCmdLineArgs() { }

    const std::unordered_set<int>& intValues() const { return intValues_; }
    const std::unordered_set<double>& dblValues() const { return dblValues_; }
    const std::unordered_set<std::string>& strValues() const {
      return strValues_;
    }
    const std::unordered_set<TestEnum> enumValues() const {
      return enumValues_;
    }
    const std::unordered_set<std::string>& formattedValues() const {
      return fmtValues_;
    }
    
    ::testing::AssertionResult checkIntValues(
        const std::unordered_set<int>& truth
    ) const {
      return check_("int", intValues(), truth);
    }
    
    ::testing::AssertionResult checkDblValues(
        const std::unordered_set<double>& truth, double tol
    ) const {
      return check_("double", dblValues(), truth, tol);
    }
    ::testing::AssertionResult checkStringValues(
	const std::unordered_set<std::string>& truth
    ) const {
      return check_("string", strValues(), truth);
    }
    ::testing::AssertionResult checkEnumValues(
        const std::unordered_set<TestEnum>& truth
    ) const {
      return check_("ValueMap", enumValues(), truth);
    }
    ::testing::AssertionResult checkFormattedValues(
	const std::unordered_set<std::string>& truth
    ) const {
      return check_("formatting function", formattedValues(), truth);
    }

  protected:
    std::unordered_set<int> intValues_;
    std::unordered_set<double> dblValues_;
    std::unordered_set<std::string> strValues_;
    std::unordered_set<TestEnum> enumValues_;
    std::unordered_set<std::string> fmtValues_;

    template <typename ValueT>
    static ::testing::AssertionResult check_(
	const std::string& name, const std::unordered_set<ValueT>& values,
	const std::unordered_set<ValueT>& truth
    ) {
      if (values != truth) {
	return ::testing::AssertionFailure()
	    << "Values for " << name << " are ["
	    << util::join(values.begin(), values.end(), ", ")
	    << "]; they should be ["
	    << util::join(truth.begin(), truth.end(), ",") << "]";
      }
      return ::testing::AssertionSuccess();
    }

    static ::testing::AssertionResult check_(
	const std::string& name, const std::unordered_set<double>& values,
	const std::unordered_set<double>& truth, double tol
    ) {
      std::vector<double> orderedValues(values.begin(), values.end());
      std::vector<double> orderedTruth(truth.begin(), truth.end());
      std::sort(orderedValues.begin(), orderedValues.end());
      std::sort(orderedTruth.begin(), orderedTruth.end());
      
      auto i = orderedValues.begin();
      auto j = orderedTruth.begin();
      bool bad = orderedValues.size() != orderedTruth.size();

      while (!bad && (i != orderedValues.end())) {
	double delta= *i - *j;
	bad= (delta < -tol) || (delta > tol);
	++i; ++j;
      }
    
      if (bad) {
	return ::testing::AssertionFailure()
	    << "Values for " << name << " are ["
	    << util::join(orderedValues.begin(), orderedValues.end(), ",")
	    << "]; they should be ["
	    << util::join(orderedTruth.begin(), orderedTruth.end(), ",")
	    << "]";
      }
      return ::testing::AssertionSuccess();
    }
  };

  class NamedSetCmdLineArgs : public SetCmdLineArgs {
  public:
    NamedSetCmdLineArgs(bool useRepeated);
  };

  class UnnamedSetCmdLineArgs: public SetCmdLineArgs {
  public:
    enum Configuration {
      SEPARATED,  // Values separated by commas in one argument
      INTEGER,    // Repeated integers in separate arguments
      DOUBLE,     // Repeated doubles in separate arguments
      STRING,     // Repeated strings in separate arguments
      TESTENUM,   // Repeated TestEnum values in separate arguments
      FORMATTED,  // Repeated formatted values in separate arguments
    };

  public:
    UnnamedSetCmdLineArgs(Configuration config);
  };

  class VectorCmdLineArgs : public AnySimpleCmdLineArgs {
  public:
    const std::vector<int>& intValues() const { return intValues_; }
    const std::vector<double>& dblValues() const { return dblValues_; }
    const std::vector<std::string>& strValues() const { return strValues_; }
    const std::vector<TestEnum> enumValues() const { return enumValues_; }
    const std::vector<std::string>& formattedValues() const {
      return fmtValues_;
    }

    ::testing::AssertionResult checkIntValues(
        const std::vector<int>& truth
    ) const {
      return check_("int", intValues(), truth);
    }
    
    ::testing::AssertionResult checkDblValues(const std::vector<double>& truth,
					      double tol) const {
      return check_("double", dblValues(), truth, tol);
    }
    ::testing::AssertionResult checkStringValues(
	const std::vector<std::string>& truth
    ) const {
      return check_("string", strValues(), truth);
    }
    
    ::testing::AssertionResult checkEnumValues(
        const std::vector<TestEnum>& truth
    ) const {
      return check_("ValueMap", enumValues(), truth);
    }
    ::testing::AssertionResult checkFormattedValues(
	const std::vector<std::string>& truth
    ) const {
      return check_("formatting function", formattedValues(), truth);
    }

  protected:
    std::vector<int> intValues_;
    std::vector<double> dblValues_;
    std::vector<std::string> strValues_;
    std::vector<TestEnum> enumValues_;
    std::vector<std::string> fmtValues_;

    template <typename Value>
    static ::testing::AssertionResult check_(const std::string& name,
					     const std::vector<Value>& values,
					     const std::vector<Value>& truth) {
      if (values != truth) {
	return ::testing::AssertionFailure()
	    << "Values for " << name << " are ["
	    << util::join(values.begin(), values.end(), ",")
	    << "]; they should be ["
	    << util::join(truth.begin(), truth.end(), ",") << "]";
      }
      return ::testing::AssertionSuccess();
    }

    static ::testing::AssertionResult check_(const std::string& name,
					     const std::vector<double>& values,
					     const std::vector<double>& truth,
					     double tol) {
      bool bad= values.size() != truth.size();
      auto i= values.begin();
      auto j= truth.begin();

      while (!bad && (i != values.end())) {
	double delta= *i - *j;
	bad= (delta < -tol) || (delta > tol);
	++i; ++j;
      }
    
      if (bad) {
	return ::testing::AssertionFailure()
	    << "Values for " << name << " are ["
	    << util::join(values.begin(), values.end(), ",")
	    << "]; they should be ["
	    << util::join(truth.begin(), truth.end(), ",") << "]";
      }
      return ::testing::AssertionSuccess();
    }
  };

  class NamedVectorCmdLineArgs : public VectorCmdLineArgs {
  public:
    NamedVectorCmdLineArgs(bool useRepeated);
  };

  class UnnamedVectorCmdLineArgs: public VectorCmdLineArgs {
  public:
    enum Configuration {
      SEPARATED,  // Values separated by commas in one argument
      INTEGER,    // Repeated integers in separate arguments
      DOUBLE,     // Repeated doubles in separate arguments
      STRING,     // Repeated strings in separate arguments
      TESTENUM,   // Repeated TestEnum values in separate arguments
      FORMATTED,  // Repeated formatted values in separate arguments
    };

  public:
    UnnamedVectorCmdLineArgs(Configuration config);
  };

  class ArbitraryCmdLineArgs : public AnySimpleCmdLineArgs {
  public:
    ArbitraryCmdLineArgs();

    int intValue() const { return i_; }
    double dblValue() const { return d_; }
    std::string strValue() const { return s_; }

  protected:
    int i_;
    double d_;
    std::string s_;
  };

  class NamedArbitraryCmdLineArgs : public ArbitraryCmdLineArgs {
  public:
    NamedArbitraryCmdLineArgs();

  private:
    void handleArgs_(CmdLineArgGenerator& args, const std::string& argName);
  };

  class UnnamedArbitraryCmdLineArgs : public ArbitraryCmdLineArgs {
  public:
    UnnamedArbitraryCmdLineArgs();

  private:
    void handleArgs_(CmdLineArgGenerator& args, const std::string& argValue);
  };

  SingleValueCmdLineArgs::SingleValueCmdLineArgs():
      AnySimpleCmdLineArgs(), i_(0), d_(0.0), s_(), e_(TestEnum::NONE), f_() {
  }

  NamedSingleValueCmdLineArgs::NamedSingleValueCmdLineArgs():
      SingleValueCmdLineArgs() {
    std::function<std::string (const std::string&)> formatFn=
        [](const std::string& value) -> std::string {
          if ((value.size() < 3) || (value.substr(0,2) != "##")) {
	    throw FormatError(value, "Must begin with \"##\"");
	  }
	  return value.substr(2);
        };

    registerNamedArg_("-i", "integer value", true, i_);
    registerNamedArg_("-d", "double value", false, d_);
    registerNamedArg_("-s", "string value", false, s_);
    registerNamedArg_("-e", "enum value", false, testEnumValueMap_(), e_);
    registerNamedArg_("-f", "formatted value", false, formatFn, f_);
  }

  NamedSingleValueInRangeCmdLineArgs::NamedSingleValueInRangeCmdLineArgs():
      SingleValueCmdLineArgs()
  {
    registerNamedArgInRange_("-i", "integer value", false, -10, 10, i_);
    registerNamedArgInRange_("-d", "double value", false, 0.0, 1.0, d_);
    registerNamedArgInRange_("-s", "string value", false, std::string("a"),
			     std::string("b"), s_);
  }

  NamedSingleValueInSetCmdLineArgs::NamedSingleValueInSetCmdLineArgs():
      SingleValueCmdLineArgs() {
    std::unordered_set<std::string> legalStrings({ "alpha", "beta", "gamma" });
    registerNamedArgInSet_("-i", "integer value", false, { 10, 20, 30, 40, 50 },
			   i_);
    registerNamedArgInSet_("-d", "double value", false,
			   {0.25, 0.5, 1.0, 2.0, 4.0 }, d_);
    registerNamedArgInSet_("-s", "string value", false, legalStrings, s_);
  }

  UnnamedSingleValueCmdLineArgs::UnnamedSingleValueCmdLineArgs():
      SingleValueCmdLineArgs() {
    std::function<std::string (const std::string&)> formatFn=
        [](const std::string& value) -> std::string {
          if ((value.size() < 3) || (value.substr(0,2) != "##")) {
	    throw FormatError(value, "Must begin with \"##\"");
	  }
	  return value.substr(2);
        };

    registerUnnamedArg_("integer value", true, i_);
    registerUnnamedArg_("double value", false, d_);
    registerUnnamedArg_("string value", false, s_);
    registerUnnamedArg_("enum value", false, testEnumValueMap_(), e_);
    registerUnnamedArg_("formatted value", false, formatFn, f_);
  }

  UnnamedSingleValueInRangeCmdLineArgs::UnnamedSingleValueInRangeCmdLineArgs():
      SingleValueCmdLineArgs() {
    registerUnnamedArgInRange_("integer value", false, -10, 10, i_);
    registerUnnamedArgInRange_("double value", false, 0.0, 1.0, d_);
    registerUnnamedArgInRange_("string value", false, std::string("a"),
			       std::string("b"), s_);
  }

  UnnamedSingleValueInSetCmdLineArgs::UnnamedSingleValueInSetCmdLineArgs():
      SingleValueCmdLineArgs() {
    std::unordered_set<std::string> legalStrings({ "alpha", "beta", "gamma" });
    registerUnnamedArgInSet_("integer value", false, { 10, 20, 30, 40, 50 },
			     i_);
    registerUnnamedArgInSet_("double value", false,
			     { 0.25, 0.5, 1.0, 2.0, 4.0 }, d_);
    registerUnnamedArgInSet_("string value", false, legalStrings, s_);
  }

  NamedSetCmdLineArgs::NamedSetCmdLineArgs(bool useRepeated) {
    std::function<std::string (const std::string&)> formatFn=
        [](const std::string& value) -> std::string {
          if ((value.size() < 3) || (value.substr(0,2) != "##")) {
	    throw FormatError(value, "Must begin with \"##\"");
	  }
	  return value.substr(2);
        };

    if (useRepeated) {
      registerNamedArg_("-i", "integer value", false, intValues_);
      registerNamedArg_("-d", "double value", false, dblValues_);
      registerNamedArg_("-s", "string value", false, strValues_);
      registerNamedArg_("-e", "enum value", false, testEnumValueMap_(),
			enumValues_);
      registerNamedArg_("-f", "formatted value", false, formatFn, fmtValues_);
    } else {
      registerNamedArg_("-i", "integer values", false, ",", false, intValues_);
      registerNamedArg_("-d", "double values", false, ",", false, dblValues_);
      registerNamedArg_("-s", "string values", false, ",", false, strValues_);
      registerNamedArg_("-e", "enum values", false, ",", false,
			testEnumValueMap_(), enumValues_);
      registerNamedArg_("-f", "formatted values", false, ",", false,
			formatFn, fmtValues_);
    }
  }

  UnnamedSetCmdLineArgs::UnnamedSetCmdLineArgs(Configuration config) {
    std::function<std::string (const std::string&)> formatFn=
        [](const std::string& value) -> std::string {
          if ((value.size() < 3) || (value.substr(0,2) != "##")) {
	    throw FormatError(value, "Must begin with \"##\"");
	  }
	  return value.substr(2);
        };

    switch(config) {
      case SEPARATED:
	registerUnnamedArg_("integer values", false, ",", intValues_);
	registerUnnamedArg_("double values", false, ",", dblValues_);
	registerUnnamedArg_("string values", false, ",", strValues_);
	registerUnnamedArg_("enum values", false, ",", testEnumValueMap_(),
			    enumValues_);
	registerUnnamedArg_("formatted value", false, ",", formatFn,
			    fmtValues_);
	break;

      case INTEGER:
	registerUnnamedArg_("integer value", false, intValues_);
	break;

      case DOUBLE:
	registerUnnamedArg_("double value", false, dblValues_);
	break;

      case STRING:
	registerUnnamedArg_("string value", false, strValues_);
	break;

      case TESTENUM:
	registerUnnamedArg_("enum value", false, testEnumValueMap_(),
			    enumValues_);
	break;

      case FORMATTED:
	registerUnnamedArg_("formatted value", false, formatFn, fmtValues_);
	break;
    }
  }

  NamedVectorCmdLineArgs::NamedVectorCmdLineArgs(bool useRepeated) {
    std::function<std::string (const std::string&)> formatFn=
        [](const std::string& value) -> std::string {
          if ((value.size() < 3) || (value.substr(0,2) != "##")) {
	    throw FormatError(value, "Must begin with \"##\"");
	  }
	  return value.substr(2);
        };

    if (useRepeated) {
      registerNamedArg_("-i", "integer value", false, intValues_);
      registerNamedArg_("-d", "double value", false, dblValues_);
      registerNamedArg_("-s", "string value", false, strValues_);
      registerNamedArg_("-e", "enum value", false, testEnumValueMap_(),
			enumValues_);
      registerNamedArg_("-f", "formatted value", false, formatFn, fmtValues_);
    } else {
      registerNamedArg_("-i", "integer values", false, ",", false, intValues_);
      registerNamedArg_("-d", "double values", false, ",", false, dblValues_);
      registerNamedArg_("-s", "string values", false, ",", false, strValues_);
      registerNamedArg_("-e", "enum values", false, ",", false,
			testEnumValueMap_(), enumValues_);
      registerNamedArg_("-f", "formatted values", false, ",", false,
			formatFn, fmtValues_);
    }
  }

  UnnamedVectorCmdLineArgs::UnnamedVectorCmdLineArgs(Configuration config) {
    std::function<std::string (const std::string&)> formatFn=
        [](const std::string& value) -> std::string {
          if ((value.size() < 3) || (value.substr(0,2) != "##")) {
	    throw FormatError(value, "Must begin with \"##\"");
	  }
	  return value.substr(2);
    };

    switch(config) {
      case SEPARATED:
	registerUnnamedArg_("integer values", false, ",", intValues_);
	registerUnnamedArg_("double values", false, ",", dblValues_);
	registerUnnamedArg_("string values", false, ",", strValues_);
	registerUnnamedArg_("enum values", false, ",", testEnumValueMap_(),
			    enumValues_);
	registerUnnamedArg_("formatted value", false, ",", formatFn,
			    fmtValues_);
	break;

      case INTEGER:
	registerUnnamedArg_("integer value", false, intValues_);
	break;

      case DOUBLE:
	registerUnnamedArg_("double value", false, dblValues_);
	break;

      case STRING:
	registerUnnamedArg_("string value", false, strValues_);
	break;

      case TESTENUM:
	registerUnnamedArg_("enum value", false, testEnumValueMap_(),
			    enumValues_);
	break;

      case FORMATTED:
	registerUnnamedArg_("formatted value", false, formatFn, fmtValues_);
	break;
    }
  }

  ArbitraryCmdLineArgs::ArbitraryCmdLineArgs():
      AnySimpleCmdLineArgs(), i_(0), d_(0.0), s_() {
    // Intentionally left blank
  }

  NamedArbitraryCmdLineArgs::NamedArbitraryCmdLineArgs():
      ArbitraryCmdLineArgs() {

    // TODO: Replace this with a bind of this to handleArgs_ without the lambda
    //       as soon as we can look up the syntax.
    registerNamedArg_(
        "-x", "data", false,
	[this](CmdLineArgGenerator& args, const std::string& argName) -> void {
	  this->handleArgs_(args, argName);
	}
    );
  }

  void NamedArbitraryCmdLineArgs::handleArgs_(CmdLineArgGenerator& args,
					      const std::string& argName) {
    i_= args.nextAsInt(argName);
    d_= args.nextAsDouble(argName);
    s_= args.next(argName);
  }

  UnnamedArbitraryCmdLineArgs::UnnamedArbitraryCmdLineArgs():
      ArbitraryCmdLineArgs() {
    registerUnnamedArg_(
        "data", false,
	[this](CmdLineArgGenerator& args, const std::string& argValue) {
	  this->handleArgs_(args, argValue);
	}
    );
  }

  void UnnamedArbitraryCmdLineArgs::handleArgs_(CmdLineArgGenerator& args,
						const std::string& argValue) {
    i_= ArgFormatter<int>::format(argValue);
    d_= args.nextAsDouble();
    s_= args.next();
  }

}

#define ARGC_FOR(args) (sizeof(args)/sizeof(const char*))-1

TEST(SimpleCmdLineArgsTests, NamedSingleValue) {
  const char* ARGV[] =
      { "some_program", "-i", "100", "-d", "-0.5", "-s", "foo", nullptr };
  const char* WITH_ENUM[] =
      { "some_program", "-i", "101", "-e", "two", "-s", "barfoo", nullptr };
  const char* WITH_FORMATTED_VALUE[] =
      { "some_program", "-i", "0", "-e", "one", "-f", "##abc", nullptr };
  const char* MISSING_REQUIRED[] =
      { "some_program", "-d", "15.1", "-s", "bar", nullptr };
  const char* UNKNOWN_ARG[] =
      { "some_program", "-i", "-3", "-d", "1.0", "-x", "foo", nullptr };
  const char* MISSING_VALUE[] =
      { "some_program", "-s", "some_string", "-d", nullptr };
  const char* INVALID_INT[] =
      { "some_program", "-d", "100.5", "-i", "bad", "-s", "foo", nullptr };
  const char* INVALID_DBL[] =
      { "some_program", "-s", "tralala", "-d", "bad", "-i", "155", nullptr };
  const char* INVALID_ENUM[] = { "some_program", "-e", "four", nullptr };
  const char* INVALID_FORMATTED_VALUE[] =
      { "some_program", "-f", "abc", nullptr };
  NamedSingleValueCmdLineArgs namedArgs;
  
  namedArgs.parse(ARGC_FOR(ARGV), const_cast<char**>(ARGV));
  EXPECT_TRUE(namedArgs.initValuesCalled());
  EXPECT_TRUE(namedArgs.checkValuesCalled());
  EXPECT_EQ(namedArgs.intValue(), 100);
  EXPECT_NEAR(namedArgs.doubleValue(), -0.5, 1e-10);
  EXPECT_EQ(namedArgs.strValue(), "foo");
  EXPECT_EQ(namedArgs.enumValue(), TestEnum::NONE);
  EXPECT_EQ(namedArgs.formattedValue(), "");

  namedArgs.reset();
  namedArgs.parse(ARGC_FOR(WITH_ENUM), const_cast<char**>(WITH_ENUM));
  EXPECT_EQ(namedArgs.intValue(), 101);
  EXPECT_NEAR(namedArgs.doubleValue(), 0.0, 1e-10);
  EXPECT_EQ(namedArgs.strValue(), "barfoo");
  EXPECT_EQ(namedArgs.enumValue(), TestEnum::TWO);
  EXPECT_EQ(namedArgs.formattedValue(), "");

  namedArgs.reset();
  namedArgs.parse(ARGC_FOR(WITH_FORMATTED_VALUE),
		  const_cast<char**>(WITH_FORMATTED_VALUE));
  EXPECT_EQ(namedArgs.intValue(), 0);
  EXPECT_NEAR(namedArgs.doubleValue(), 0.0, 1e-10);
  EXPECT_EQ(namedArgs.strValue(), "");
  EXPECT_EQ(namedArgs.enumValue(), TestEnum::ONE);
  EXPECT_EQ(namedArgs.formattedValue(), "abc");

  namedArgs.reset();
  EXPECT_THROW(namedArgs.parse(ARGC_FOR(MISSING_REQUIRED),
			       const_cast<char**>(MISSING_REQUIRED)),
	       RequiredCmdLineArgMissingError);
  EXPECT_THROW(namedArgs.parse(ARGC_FOR(UNKNOWN_ARG),
			       const_cast<char**>(UNKNOWN_ARG)),
	       UnknownCmdLineArgError);
  EXPECT_THROW(namedArgs.parse(ARGC_FOR(MISSING_VALUE),
			       const_cast<char**>(MISSING_VALUE)),
	       ValueMissingError);
  EXPECT_THROW(namedArgs.parse(ARGC_FOR(INVALID_INT),
			       const_cast<char**>(INVALID_INT)),
	       IllegalValueError);
  EXPECT_THROW(namedArgs.parse(ARGC_FOR(INVALID_DBL),
			       const_cast<char**>(INVALID_DBL)),
	       IllegalValueError);
  EXPECT_THROW(namedArgs.parse(ARGC_FOR(INVALID_ENUM),
			       const_cast<char**>(INVALID_ENUM)),
	       IllegalValueError);
  EXPECT_THROW(namedArgs.parse(ARGC_FOR(INVALID_FORMATTED_VALUE),
			       const_cast<char**>(INVALID_FORMATTED_VALUE)),
	       IllegalValueError);
}

TEST(SimpleCmdLineArgsTests, UnnamedSingleValue) {
  const char* ARGV[] = { "some_program", "151", "1.25", "foobar", nullptr };
  const char* WITH_ENUM[] =
      { "some_program", "101", "0.5", "barfoo", "three", nullptr };
  const char* WITH_FORMATTED_VALUE[] =
      { "some_program", "102", "0.99", "baz", "two", "##ABC", nullptr };
  const char* MISSING_REQUIRED[] = { "some_program", nullptr };
  const char* TOO_MANY_ARG[] =
      { "some_program", "3", "1.0", "foo", "three", "##ABC", "bar", nullptr };
  const char* INVALID_INT[] =
      { "some_program", "bad", "111.5", "foo", nullptr };
  const char* INVALID_DBL[] =
      { "some_program", "15", "bad", "bar", nullptr };
  const char* INVALID_ENUM[] =
      { "some_program", "15", "0.1", "bar", "bad", nullptr };
  const char* INVALID_FORMATTED_VALUE[] =
      { "some_program", "15", "0.1", "bar", "one", "ABC", nullptr };
  UnnamedSingleValueCmdLineArgs args;
  
  args.parse(ARGC_FOR(ARGV), const_cast<char**>(ARGV));
  EXPECT_TRUE(args.initValuesCalled());
  EXPECT_TRUE(args.checkValuesCalled());
  EXPECT_EQ(args.intValue(), 151);
  EXPECT_NEAR(args.doubleValue(), 1.25, 1e-10);
  EXPECT_EQ(args.strValue(), "foobar");
  EXPECT_EQ(args.enumValue(), TestEnum::NONE);
  EXPECT_EQ(args.formattedValue(), "");

  args.reset();
  args.parse(ARGC_FOR(WITH_ENUM), const_cast<char**>(WITH_ENUM));
  EXPECT_EQ(args.intValue(), 101);
  EXPECT_NEAR(args.doubleValue(), 0.5, 1e-10);
  EXPECT_EQ(args.strValue(), "barfoo");
  EXPECT_EQ(args.enumValue(), TestEnum::THREE);
  EXPECT_EQ(args.formattedValue(), "");

  args.reset();
  args.parse(ARGC_FOR(WITH_FORMATTED_VALUE),
	     const_cast<char**>(WITH_FORMATTED_VALUE));
  EXPECT_EQ(args.intValue(), 102);
  EXPECT_NEAR(args.doubleValue(), 0.99, 1e-10);
  EXPECT_EQ(args.strValue(), "baz");
  EXPECT_EQ(args.enumValue(), TestEnum::TWO);
  EXPECT_EQ(args.formattedValue(), "ABC");

  args.reset();
  EXPECT_THROW(args.parse(ARGC_FOR(MISSING_REQUIRED),
			  const_cast<char**>(MISSING_REQUIRED)),
	       RequiredCmdLineArgMissingError);
  EXPECT_THROW(args.parse(ARGC_FOR(TOO_MANY_ARG),
			  const_cast<char**>(TOO_MANY_ARG)),
	       TooManyCmdLineArgsError);
  EXPECT_THROW(args.parse(ARGC_FOR(INVALID_INT),
			  const_cast<char**>(INVALID_INT)),
	       IllegalValueError);
  EXPECT_THROW(args.parse(ARGC_FOR(INVALID_DBL),
			  const_cast<char**>(INVALID_DBL)),
	       IllegalValueError);
  EXPECT_THROW(args.parse(ARGC_FOR(INVALID_ENUM),
			  const_cast<char**>(INVALID_ENUM)),
	       IllegalValueError);
  EXPECT_THROW(args.parse(ARGC_FOR(INVALID_FORMATTED_VALUE),
			  const_cast<char**>(INVALID_FORMATTED_VALUE)),
	       IllegalValueError);
}

TEST(SimpleCmdLineArgsTests, NamedSingleValueInRange) {
  const char* ARGV[] =
      { "some_program", "-i", "1", "-d", "0.5", "-s", "abc", nullptr };
  const char* INVALID_INT[] =
      { "some_program", "-d", "0.5", "-i", "bad", "-s", "foo", nullptr };
  const char* INVALID_DBL[] =
      { "some_program", "-s", "able", "-d", "bad", "-i", "9", nullptr };
  const char* INT_OUT_OF_RANGE[] =
      { "some_program", "-i", "11", "-d", "0.5", "-s", "abc", nullptr };
  const char* DBL_OUT_OF_RANGE[] =
      { "some_program", "-i", "-5", "-d", "-0.5", "-s", "abc", nullptr };
  const char* STR_OUT_OF_RANGE[] =
      { "some_program", "-i", "-5", "-d", "0.99", "-s", "def", nullptr };
  NamedSingleValueInRangeCmdLineArgs args;

  args.parse(ARGC_FOR(ARGV), const_cast<char**>(ARGV));
  EXPECT_EQ(args.intValue(), 1);
  EXPECT_NEAR(args.doubleValue(), 0.5, 1e-10);
  EXPECT_EQ(args.strValue(), "abc");

  args.reset();
  EXPECT_THROW(args.parse(ARGC_FOR(INVALID_INT),
			  const_cast<char**>(INVALID_INT)),
	       IllegalValueError);
  args.reset();
  EXPECT_THROW(args.parse(ARGC_FOR(INVALID_DBL),
			  const_cast<char**>(INVALID_DBL)),
	       IllegalValueError);
  args.reset();
  EXPECT_THROW(args.parse(ARGC_FOR(INT_OUT_OF_RANGE),
			  const_cast<char**>(INT_OUT_OF_RANGE)),
	       IllegalValueError);
  args.reset();
  EXPECT_THROW(args.parse(ARGC_FOR(DBL_OUT_OF_RANGE),
			  const_cast<char**>(DBL_OUT_OF_RANGE)),
	       IllegalValueError);
  args.reset();
  EXPECT_THROW(args.parse(ARGC_FOR(STR_OUT_OF_RANGE),
			  const_cast<char**>(STR_OUT_OF_RANGE)),
	       IllegalValueError);
}

TEST(SimpleCmdLineArgsTests, NamedSingleValueInSet) {
  const char* ARGV[] =
      { "some_program", "-i", "20", "-d", "2.0", "-s", "beta", nullptr };
  const char* INT_OUT_OF_SET[] =
      { "some_program", "-i", "11", "-d", "0.5", "-s", "beta", nullptr };
  const char* DBL_OUT_OF_SET[] =
      { "some_program", "-i", "30", "-d", "-0.5", "-s", "alpha", nullptr };
  const char* STR_OUT_OF_SET[] =
      { "some_program", "-i", "30", "-d", "0.25", "-s", "def", nullptr };
  NamedSingleValueInSetCmdLineArgs args;

  args.parse(ARGC_FOR(ARGV), const_cast<char**>(ARGV));
  EXPECT_EQ(args.intValue(), 20);
  EXPECT_NEAR(args.doubleValue(), 2.0, 1e-10);
  EXPECT_EQ(args.strValue(), "beta");

  args.reset();
  EXPECT_THROW(args.parse(ARGC_FOR(INT_OUT_OF_SET),
			  const_cast<char**>(INT_OUT_OF_SET)),
	       IllegalValueError);
  args.reset();
  EXPECT_THROW(args.parse(ARGC_FOR(DBL_OUT_OF_SET),
			  const_cast<char**>(DBL_OUT_OF_SET)),
	       IllegalValueError);
  args.reset();
  EXPECT_THROW(args.parse(ARGC_FOR(STR_OUT_OF_SET),
			  const_cast<char**>(STR_OUT_OF_SET)),
	       IllegalValueError);
}

TEST(SimpleCmdLineArgsTests, UnnamedSingleValueInRange) {

}

TEST(SimpleCmdLineArgsTests, UnnamedSingleValueInSet) {

}

TEST(SimpleCmdLineArgsTests, NamedRepeatedVectorArgs) {
  const char* ARGV[] = {
      "some_program", "-i", "1", "-d", "0.5", "-i", "3", "-d", "-0.5", "-s",
      "abc", "-s", "def", "-e", "three", "-i", "2", "-e", "two", "-f",
      "##ZZZ", "-e", "one", nullptr
  };
  NamedVectorCmdLineArgs args(true);

  args.parse(ARGC_FOR(ARGV), const_cast<char**>(ARGV));
  EXPECT_TRUE(args.checkIntValues({ 1, 3, 2}));
  EXPECT_TRUE(args.checkDblValues({ 0.5, -0.5}, 1e-10));
  EXPECT_TRUE(args.checkStringValues({ "abc", "def" }));
  EXPECT_TRUE(args.checkEnumValues({ TestEnum::THREE, TestEnum::TWO,
	                             TestEnum::ONE }));
  EXPECT_TRUE(args.checkFormattedValues({ "ZZZ" }));
}

TEST(SimpleCmdLineArgsTests, NamedSeparatedVectorArgs) {
  const char* ARGV[] = {
      "some_program", "-s", "a,b,a,c,d", "-i", "3,2,7,4", "-d", "-1.0", "-e",
      "one,three,three,two", "-f", "##XYZ,##AABBCC", nullptr
  };
  NamedVectorCmdLineArgs args(false);

  args.parse(ARGC_FOR(ARGV), const_cast<char**>(ARGV));
  EXPECT_TRUE(args.checkIntValues({3, 2, 7, 4}));
  EXPECT_TRUE(args.checkDblValues({-1.0}, 1e-10));
  EXPECT_TRUE(args.checkStringValues({ "a", "b", "a", "c", "d" }));
  EXPECT_TRUE(args.checkEnumValues({ TestEnum::ONE, TestEnum::THREE,
	                             TestEnum::THREE, TestEnum::TWO }));
  EXPECT_TRUE(args.checkFormattedValues({ "XYZ", "AABBCC" }));
}

TEST(SimpleCmdLineArgsTests, UnnamedRepeatedVectorArgs) {
  const char* INT_ARGV[] = { "some_program", "12", "19", "27", "14", nullptr };
  const char* DBL_ARGV[] = { "some_program", "17.1", "8.6", nullptr };
  const char* STR_ARGV[] =
      { "some_program", "i", "you", "he", "she", "it", nullptr };
  const char* ENUM_ARGV[] = { "some_program", "two", nullptr };
  const char* FMT_ARGV[] = { "some_program", "##a", "##B", "##c", nullptr };
  UnnamedVectorCmdLineArgs intArgs(UnnamedVectorCmdLineArgs::INTEGER);
  UnnamedVectorCmdLineArgs dblArgs(UnnamedVectorCmdLineArgs::DOUBLE);
  UnnamedVectorCmdLineArgs strArgs(UnnamedVectorCmdLineArgs::STRING);
  UnnamedVectorCmdLineArgs enumArgs(UnnamedVectorCmdLineArgs::TESTENUM);
  UnnamedVectorCmdLineArgs fmtArgs(UnnamedVectorCmdLineArgs::FORMATTED);

  intArgs.parse(ARGC_FOR(INT_ARGV), const_cast<char**>(INT_ARGV));
  EXPECT_TRUE(intArgs.checkIntValues({12, 19, 27, 14}));

  dblArgs.parse(ARGC_FOR(DBL_ARGV), const_cast<char**>(DBL_ARGV));
  EXPECT_TRUE(dblArgs.checkDblValues({17.1, 8.6}, 1e-10));

  strArgs.parse(ARGC_FOR(STR_ARGV), const_cast<char**>(STR_ARGV));
  EXPECT_TRUE(strArgs.checkStringValues({ "i", "you", "he", "she", "it" }));

  enumArgs.parse(ARGC_FOR(ENUM_ARGV), const_cast<char**>(ENUM_ARGV));
  EXPECT_TRUE(enumArgs.checkEnumValues({ TestEnum::TWO }));

  fmtArgs.parse(ARGC_FOR(FMT_ARGV), const_cast<char**>(FMT_ARGV));
  EXPECT_TRUE(fmtArgs.checkFormattedValues({ "a", "B", "c" }));
}

TEST(SimpleCmdLineArgsTests, UnnamedSeparatedVectorArgs) {
  const char* ARGV[] = {
      "some_program", "0,5,0,11,4", "0.5,-1.5,25.5", "red,green,blue",
      "three,two,three,two", "##a", nullptr
  };
  UnnamedVectorCmdLineArgs args(UnnamedVectorCmdLineArgs::SEPARATED);

  args.parse(ARGC_FOR(ARGV), const_cast<char**>(ARGV));
  EXPECT_TRUE(args.checkIntValues({0, 5, 0, 11, 4}));
  EXPECT_TRUE(args.checkDblValues({0.5, -1.5, 25.5}, 1e-10));
  EXPECT_TRUE(args.checkStringValues({"red", "green", "blue"}));
  EXPECT_TRUE(args.checkEnumValues({ TestEnum::THREE, TestEnum::TWO,
	                             TestEnum::THREE, TestEnum::TWO }));
  EXPECT_TRUE(args.checkFormattedValues({ "a" }));
}

TEST(SimpleCmdLineArgsTests, NamedRepeatedSetArgs) {
  const char* ARGV[] = {
      "some_program", "-i", "1", "-d", "0.5", "-i", "3", "-d", "-0.5", "-s",
      "abc", "-s", "def", "-e", "three", "-i", "1", "-e", "two", "-f", "##ZZZ",
      "-s", "def", "-e", "one", "-s", "ghijkl", nullptr
  };
  NamedSetCmdLineArgs args(true);

  args.parse(ARGC_FOR(ARGV), const_cast<char**>(ARGV));
  EXPECT_TRUE(args.checkIntValues({ 1, 3 }));
  EXPECT_TRUE(args.checkDblValues({ 0.5, -0.5}, 1e-10));
  EXPECT_TRUE(args.checkStringValues({ "abc", "def", "ghijkl" }));
  EXPECT_TRUE(args.checkEnumValues({ TestEnum::ONE, TestEnum::TWO,
	                             TestEnum::THREE}));
  EXPECT_TRUE(args.checkFormattedValues({ "ZZZ" }));
}

TEST(SimpleCmdLineArgsTests, NamedSeparatedSetArgs) {
  const char* ARGV[] = {
    "some_program", "-s", "a,b,a,c,d", "-i", "3,2,7,4", "-d", "-1.0", "-e",
    "one,three,three,two", "-f", "##XYZ,##AABBCC", nullptr
  };
  NamedSetCmdLineArgs args(false);

  args.parse(ARGC_FOR(ARGV), const_cast<char**>(ARGV));
  EXPECT_TRUE(args.checkIntValues({2, 3, 4, 7}));
  EXPECT_TRUE(args.checkDblValues({-1.0}, 1e-10));
  EXPECT_TRUE(args.checkStringValues({ "a", "b", "c", "d" }));
  EXPECT_TRUE(args.checkEnumValues({ TestEnum::ONE, TestEnum::TWO,
	                             TestEnum::THREE }));
  EXPECT_TRUE(args.checkFormattedValues({ "XYZ", "AABBCC" }));
}

TEST(SimpleCmdLineArgsTests, UnnamedRepeatedSetArgs) {
  const char* INT_ARGV[] = { "some_program", "12", "19", "27", "14", nullptr };
  const char* DBL_ARGV[] = { "some_program", "17.1", "8.6", nullptr };
  const char* STR_ARGV[] =
      { "some_program", "i", "you", "he", "she", "it", nullptr };
  const char* ENUM_ARGV[] = { "some_program", "two", nullptr };
  const char* FMT_ARGV[] = { "some_program", "##a", "##B", "##c", nullptr };
  UnnamedSetCmdLineArgs intArgs(UnnamedSetCmdLineArgs::INTEGER);
  UnnamedSetCmdLineArgs dblArgs(UnnamedSetCmdLineArgs::DOUBLE);
  UnnamedSetCmdLineArgs strArgs(UnnamedSetCmdLineArgs::STRING);
  UnnamedSetCmdLineArgs enumArgs(UnnamedSetCmdLineArgs::TESTENUM);
  UnnamedSetCmdLineArgs fmtArgs(UnnamedSetCmdLineArgs::FORMATTED);

  intArgs.parse(ARGC_FOR(INT_ARGV), const_cast<char**>(INT_ARGV));
  EXPECT_TRUE(intArgs.checkIntValues({12, 14, 19, 27 }));

  dblArgs.parse(ARGC_FOR(DBL_ARGV), const_cast<char**>(DBL_ARGV));
  EXPECT_TRUE(dblArgs.checkDblValues({8.6, 17.1}, 1e-10));

  strArgs.parse(ARGC_FOR(STR_ARGV), const_cast<char**>(STR_ARGV));
  EXPECT_TRUE(strArgs.checkStringValues({ "he", "i", "it", "she", "you" }));

  enumArgs.parse(ARGC_FOR(ENUM_ARGV), const_cast<char**>(ENUM_ARGV));
  EXPECT_TRUE(enumArgs.checkEnumValues({ TestEnum::TWO }));

  fmtArgs.parse(ARGC_FOR(FMT_ARGV), const_cast<char**>(FMT_ARGV));
  EXPECT_TRUE(fmtArgs.checkFormattedValues({ "B", "a", "c" }));
}

TEST(SimpleCmdLineArgsTests, UnnamedSeparatedSetArgs) {
  const char* ARGV[] = {
      "some_program", "0,5,0,11,4", "0.5,-1.5,25.5", "red,green,blue",
      "three,two,three,two", "##a", nullptr
  };
  UnnamedSetCmdLineArgs args(UnnamedSetCmdLineArgs::SEPARATED);

  args.parse(ARGC_FOR(ARGV), const_cast<char**>(ARGV));
  EXPECT_TRUE(args.checkIntValues({0, 5, 11, 4}));
  EXPECT_TRUE(args.checkDblValues({-1.5, 0.5, 25.5}, 1e-10));
  EXPECT_TRUE(args.checkStringValues({"blue", "green", "red"}));
  EXPECT_TRUE(args.checkEnumValues({ TestEnum::TWO, TestEnum::THREE, }));
  EXPECT_TRUE(args.checkFormattedValues({ "a" }));
}

TEST(SimpleCmdLineArgsTests, NamedArbitraryCmdLineArgs) {
  const char* ARGV[] =
      { "some_program", "-x", "999", "999.999", "plugh", nullptr };
  const char* BAD[] =
      { "some_program", "-x", "999", "bad", "plugh", nullptr };
  NamedArbitraryCmdLineArgs args;

  args.parse(ARGC_FOR(ARGV), const_cast<char**>(ARGV));
  EXPECT_EQ(args.intValue(), 999);
  EXPECT_NEAR(args.dblValue(), 999.999, 1e-10);
  EXPECT_EQ(args.strValue(), "plugh");

  EXPECT_THROW(args.parse(ARGC_FOR(BAD), const_cast<char**>(BAD)),
		    IllegalValueError);
}

TEST(SimpleCmdLineArgsTests, UnnamedArbitraryCmdLineArgs) {
  const char* ARGV[] = { "some_program", "999", "999.999", "plugh", nullptr };
  const char* BAD[] = { "some_program", "999", "bad", "plugh", nullptr };
  UnnamedArbitraryCmdLineArgs args;

  args.parse(ARGC_FOR(ARGV), const_cast<char**>(ARGV));
  EXPECT_EQ(args.intValue(), 999);
  EXPECT_NEAR(args.dblValue(), 999.999, 1e-10);
  EXPECT_EQ(args.strValue(), "plugh");

  EXPECT_THROW(args.parse(ARGC_FOR(BAD), const_cast<char**>(BAD)),
		    IllegalValueError);
}

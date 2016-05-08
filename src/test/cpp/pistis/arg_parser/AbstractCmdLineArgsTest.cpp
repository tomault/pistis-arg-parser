/** @file AbstractCmdLineArgsTest.cpp
 *
 *  Unit tests for pistis::arg_parser::AbstractCmdLineArgs.
 */

#include <pistis/arg_parser/AbstractCmdLineArgs.hpp>
#include <pistis/arg_parser/TooManyCmdLineArgsError.hpp>
#include <pistis/arg_parser/UnknownCmdLineArgError.hpp>
#include <pistis/util/StringUtil.hpp>
#include <gtest/gtest.h>
#include <set>
#include <vector>

using namespace pistis::arg_parser;
using namespace pistis::exceptions;
namespace util = pistis::util;

namespace {
  class TestCmdLineArgs : public AbstractCmdLineArgs {
  public:
    TestCmdLineArgs():
        AbstractCmdLineArgs(), initCalled_(false), checkCalled_(false),
	initArgc_(0), initArgv_(nullptr), namedArgs_(), unnamedArgs_(),
	recognizedNames_({ "-a", "-b", "-c" }), maxUnnamedArgs_(4) {
      // Intentionally left blank
    }

    bool initCalled() const { return initCalled_; }
    bool checkCalled() const { return checkCalled_; }
    int  initArgc() const { return initArgc_; }
    char** initArgv() const { return initArgv_; }
    const std::vector<std::string>& namedArgs() const {
      return namedArgs_;
    }
    const std::vector<std::string>& unnamedArgs() const {
      return unnamedArgs_;
    }

  protected:
    virtual void init_(int argc, char** argv) {
      initCalled_= true;
      initArgc_= argc;
      initArgv_= argv;
    }
    
    virtual bool handleNamedArg_(CmdLineArgGenerator& args,
				 const std::string& arg) {
      if (AbstractCmdLineArgs::handleNamedArg_(args, arg)) {
	return true;
      } else if (recognizedNames_.find(arg) != recognizedNames_.end()) {
	namedArgs_.push_back(arg);
	return true;
      } else {
	return false;
      }
    }
    
    virtual bool handleUnnamedArg_(CmdLineArgGenerator& args,
				   const std::string& value) {
      if (AbstractCmdLineArgs::handleUnnamedArg_(args, value)) {
	return true;
      } else if (unnamedArgs_.size() < maxUnnamedArgs_) {
	unnamedArgs_.push_back(value);
	return true;
      } else {
	return false;
      }
    }

    virtual void check_(const std::string& appName) {
      checkCalled_= true;
    }

  private:
    bool initCalled_;
    bool checkCalled_;
    int initArgc_;
    char** initArgv_;
    std::vector<std::string> namedArgs_;
    std::vector<std::string> unnamedArgs_;
    std::set<std::string> recognizedNames_;
    int maxUnnamedArgs_;
  };

  ::testing::AssertionResult checkVector(
      const std::string& what, const std::vector<std::string>& names,
      const std::vector<std::string>& truth
  ) {
    auto i= names.begin();
    auto j= truth.begin();

    while ((i != names.end()) && (j != truth.end())) {
      if (*i != *j) {
	break;
      }
      ++i; ++j;
    }
    if ((i != names.end()) || (j != truth.end())) {
      return ::testing::AssertionFailure()
          << "List of " << what << " is incorrect.  The list is ["
	  << util::join(names.begin(), names.end(), ", ")
	  << "].  It should be ["
	  << util::join(truth.begin(), truth.end(), ", ") << "].";
    }
    return ::testing::AssertionSuccess();
  }
}

TEST(AbstractCmdLineArgsTest, ParseSuccessfully) {
  const char* ARGV[] = { "MyApplication", "-a", "someValue", "-c",
			 "anotherValue", "-b", "thirdValue", "filename.txt",
			 nullptr };
  const int ARGC= sizeof(ARGV)/sizeof(char*) - 1;
  TestCmdLineArgs args;
  std::string errMsg;

  args.parse(ARGC, const_cast<char**>(ARGV));
  EXPECT_TRUE(args.initCalled());
  EXPECT_EQ(args.initArgc(), ARGC);
  EXPECT_EQ(args.initArgv(), ARGV);
  EXPECT_TRUE(args.checkCalled());

  EXPECT_TRUE(checkVector("named arguments", args.namedArgs(),
			  { "-a", "-c", "-b" }));

  EXPECT_TRUE(checkVector("unnamed arguments", args.unnamedArgs(),
			  { "someValue", "anotherValue", "thirdValue",
			    "filename.txt"}));
}

TEST(AbstractCmdLineArgsTest, ParseUnknownArg) {
  const char* ARGV[] = { "MyApplication", "-a", "someValue", "-d",
			 "lastIsBad", "-b", "thirdValue", "filename.txt",
			 nullptr };
  const int ARGC= sizeof(ARGV)/sizeof(char*) - 1;
  TestCmdLineArgs args;

  EXPECT_THROW(args.parse(ARGC, const_cast<char**>(ARGV)),
	       UnknownCmdLineArgError);
}

TEST(AbstractCmdLineArgsTest, ParseTooManyArgs) {
  const char* ARGV[] = { "MyApplication", "one", "two", "three", "four",
			 "five", nullptr };
  const int ARGC= sizeof(ARGV)/sizeof(char*) - 1;
  TestCmdLineArgs args;

  EXPECT_THROW(args.parse(ARGC, const_cast<char**>(ARGV)),
	       TooManyCmdLineArgsError);
}

/** @file CmdLineArgGeneratorTests.cpp
 *
 *  Unit tests for pistis::arg_parser::CmdLineArgGenerator.
 */

#include <pistis/arg_parser/CmdLineArgGenerator.hpp>
#include <pistis/arg_parser/IllegalValueError.hpp>
#include <pistis/arg_parser/ValueMissingError.hpp>
#include <gtest/gtest.h>
#include <sstream>

using namespace pistis::arg_parser;

TEST(CmdLineArgGeneratorTests, Construction) {
  const char* ARGV[] =
      { "some/path/to/MyApplication", "-p", "100", "filename.txt", NULL };
  const int ARGC = sizeof(ARGV)/sizeof(char*)-1;
  CmdLineArgGenerator args(ARGC, const_cast<char**>(ARGV));

  EXPECT_EQ(args.appName(), "some/path/to/MyApplication");
  EXPECT_EQ(args.numArgs(), ARGC-1);
  EXPECT_EQ(args.remaining(), ARGC-1);
}

TEST(CmdLineArgGeneratorTests, CurrentAndNext) {
  const char* ARGV[] =
      { "some/path/to/MyApplication", "-p", "100", "filename.txt", nullptr };
  const int ARGC = sizeof(ARGV)/sizeof(char*)-1;
  CmdLineArgGenerator args(ARGC, const_cast<char**>(ARGV));
  char** pp= const_cast<char**>(ARGV+1);
  int remaining= ARGC-1;

  while (*pp) {
    std::string arg= args.current();
    EXPECT_EQ(arg, *pp);
    EXPECT_EQ(args.remaining(), remaining);

    std::string nextArg= args.next();
    --remaining;

    EXPECT_EQ(nextArg, *pp);
    EXPECT_EQ(args.remaining(), remaining);
    ++pp;
  }

  EXPECT_EQ(args.remaining(), 0);
  EXPECT_THROW(args.current(), ValueMissingError);
  EXPECT_THROW(args.next(), ValueMissingError);
}

TEST(CmdLineArgGeneratorTests, NextInSet) {
  const char* ARGV[] =
      { "some/path/to/MyApplication", "goodValue", "badValue", nullptr };
  const int ARGC = sizeof(ARGV)/sizeof(char*)-1;
  CmdLineArgGenerator args(ARGC, const_cast<char**>(ARGV));
  
  EXPECT_EQ(args.nextInSet("-p", std::set<std::string>({ "someValue", "anotherValue", "goodValue" })), "goodValue");
  EXPECT_THROW(args.nextInSet(std::set<std::string>({ "someValue", "anotherValue", "goodValue" })), IllegalValueError);
  EXPECT_EQ(args.current(), "badValue");
}

TEST(CmdLineArgGeneratorTests, NextAsInt) {
  const char* ARGV[] =
      { "some/path/to/MyApplication", "100", "badValue", nullptr };
  const int ARGC = sizeof(ARGV)/sizeof(char*)-1;
  CmdLineArgGenerator args(ARGC, const_cast<char**>(ARGV));
  
  EXPECT_EQ(args.nextAsInt("-arg"), 100);
  EXPECT_THROW(args.nextAsInt(), IllegalValueError);
  EXPECT_EQ(args.current(), "badValue");
}

TEST(CmdLineArgGeneratorTests, NextAsIntInRange) {
  const char* ARGV[] =
      { "some/path/to/MyApplication", "100", "1000", "badValue", nullptr };
  const int ARGC = sizeof(ARGV)/sizeof(char*)-1;
  CmdLineArgGenerator args(ARGC, const_cast<char**>(ARGV));
  
  EXPECT_EQ(args.nextAsIntInRange("-arg", 0, 200), 100);
  EXPECT_THROW(args.nextAsIntInRange(0, 200), IllegalValueError);
  EXPECT_EQ(args.nextAsIntInRange("-arg", 0, 1000), 1000);
  EXPECT_THROW(args.nextAsIntInRange("-arg", 0, 1000), IllegalValueError);
  EXPECT_EQ(args.current(), "badValue");
}

TEST(CmdLineArgGeneratorTests, NextAsUInt) {
  const char* ARGV[] =
      { "some/path/to/MyApplication", "100", "-100", nullptr };
  const int ARGC = sizeof(ARGV)/sizeof(char*)-1;
  CmdLineArgGenerator args(ARGC, const_cast<char**>(ARGV));
  
  EXPECT_EQ(args.nextAsUInt("-arg"), 100);
  EXPECT_THROW(args.nextAsUInt(), IllegalValueError);
  EXPECT_EQ(args.current(), "-100");
}

TEST(CmdLineArgGeneratorTests, NextAsUIntInRange) {
  const char* ARGV[] =
      { "some/path/to/MyApplication", "100", "1000", "-1", nullptr };
  const int ARGC = sizeof(ARGV)/sizeof(char*)-1;
  CmdLineArgGenerator args(ARGC, const_cast<char**>(ARGV));
  
  EXPECT_EQ(args.nextAsUIntInRange("-arg", 0, 200), 100);
  EXPECT_THROW(args.nextAsUIntInRange(0, 200), IllegalValueError);
  EXPECT_EQ(args.nextAsUIntInRange("-arg", 0, 1000), 1000);
  EXPECT_THROW(args.nextAsUIntInRange("-arg", 0, 1000), IllegalValueError);
  EXPECT_EQ(args.current(), "-1");
}

TEST(CmdLineArgGeneratorTests, NextAsDouble) {
  const char* ARGV[] =
      { "some/path/to/MyApplication", "1.5", "badValue", nullptr };
  const int ARGC = sizeof(ARGV)/sizeof(char*)-1;
  CmdLineArgGenerator args(ARGC, const_cast<char**>(ARGV));
  
  EXPECT_NEAR(args.nextAsDouble("-arg"), 1.5, 1e-10);
  EXPECT_THROW(args.nextAsDouble(), IllegalValueError);
  EXPECT_EQ(args.current(), "badValue");
}

TEST(CmdLineArgGeneratorTests, NextAsDoubleInRange) {
  const char* ARGV[] =
      { "some/path/to/MyApplication", "1.5", "-1.5", "badValue", nullptr };
  const int ARGC = sizeof(ARGV)/sizeof(char*)-1;
  CmdLineArgGenerator args(ARGC, const_cast<char**>(ARGV));
  
  EXPECT_NEAR(args.nextAsDoubleInRange("-arg", -1.0, 2.0), 1.5, 1e-10);
  EXPECT_THROW(args.nextAsDoubleInRange(-1.0, 2.0), IllegalValueError);
  EXPECT_NEAR(args.nextAsDoubleInRange("-arg", -2.0, 2.0), -1.5, 1e-10);
  EXPECT_THROW(args.nextAsDoubleInRange("-arg", -2.0, 2.0), IllegalValueError);
  EXPECT_EQ(args.current(), "badValue");
}

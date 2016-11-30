#include <gtest/gtest.h>

#include "ant/util/path_util.h"

namespace ant {

TEST(TestPathUtil, BaseNameTest) {
  ASSERT_EQ(".", BaseName(""));
  ASSERT_EQ(".", BaseName("."));
  ASSERT_EQ("..", BaseName(".."));
  ASSERT_EQ("/", BaseName("/"));
  ASSERT_EQ("/", BaseName("//"));
  ASSERT_EQ("a", BaseName("a"));
  ASSERT_EQ("ab", BaseName("ab"));
  ASSERT_EQ("ab", BaseName("ab/"));
  ASSERT_EQ("cd", BaseName("ab/cd"));
  ASSERT_EQ("ab", BaseName("/ab"));
  ASSERT_EQ("ab", BaseName("/ab///"));
  ASSERT_EQ("cd", BaseName("/ab/cd"));
}

TEST(TestPathUtil, DirNameTest) {
  ASSERT_EQ(".", DirName(""));
  ASSERT_EQ(".", DirName("."));
  ASSERT_EQ(".", DirName(".."));
  ASSERT_EQ("/", DirName("/"));
  ASSERT_EQ("//", DirName("//"));
  ASSERT_EQ(".", DirName("a"));
  ASSERT_EQ(".", DirName("ab"));
  ASSERT_EQ(".", DirName("ab/"));
  ASSERT_EQ("ab", DirName("ab/cd"));
  ASSERT_EQ("/", DirName("/ab"));
  ASSERT_EQ("/", DirName("/ab///"));
  ASSERT_EQ("/ab", DirName("/ab/cd"));
}

} // namespace ant

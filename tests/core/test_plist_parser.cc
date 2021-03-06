/**
 * This file is part of the "FnordMetric" project
 *   Copyright (c) 2016 Paul Asmuth <paul@asmuth.com>
 *
 * FnordTable is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License v3.0. You should have received a
 * copy of the GNU General Public License along with this program. If not, see
 * <http://www.gnu.org/licenses/>.
 */
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <iostream>
#include <plist/plist_parser.h>

using namespace plist;

#define EXPECT(X) \
    if (!(X)) { \
      std::cerr << "ERROR: expectation failed: " << #X << " on line " << __LINE__ <<  std::endl; \
      std::exit(1); \
    }

#define EXPECT_EQ(A, B) EXPECT((A) == (B))

#define EXPECT_STREQ(A, B) EXPECT(std::string(A) == std::string(B))

void test_parse_simple() {
  std::string confstr =
      R"(
        prop0: 1337;
        prop1: "hello";
        prop2: 'world';
      )";

  PropertyListParser parser(confstr.c_str(), confstr.size());
  PropertyList plist;
  EXPECT(parser.parse(&plist) == true);

  EXPECT_EQ(plist.size(), 3);
  EXPECT_STREQ(plist[0].name, "prop0");
  EXPECT_EQ(plist[0].size(), 1);
  EXPECT_STREQ(plist[0][0], "1337");
  EXPECT_STREQ(plist[1].name, "prop1");
  EXPECT_EQ(plist[1].size(), 1);
  EXPECT_STREQ(plist[1][0], "hello");
  EXPECT_STREQ(plist[2].name, "prop2");
  EXPECT_EQ(plist[2].size(), 1);
  EXPECT_STREQ(plist[2][0], "world");
}

void test_parse_element() {
  std::string confstr =
      R"(
        blah: xxx;

        elem {
          prop0: 1337;
          prop1: "hello";
          prop2: 'world';
        }
      )";

  PropertyListParser parser(confstr.c_str(), confstr.size());
  PropertyList plist;
  EXPECT(parser.parse(&plist) == true);

  EXPECT_EQ(plist.size(), 2);
  EXPECT_STREQ(plist[0].name, "blah");
  EXPECT_EQ(plist[0].size(), 1);
  EXPECT_STREQ(plist[0][0], "xxx");

  EXPECT_STREQ(plist[1].name, "elem");
  auto elem = plist[1].child.get();

  EXPECT_EQ(elem->size(), 3);
  EXPECT_STREQ((*elem)[0].name, "prop0");
  EXPECT_EQ((*elem)[0].size(), 1);
  EXPECT_STREQ((*elem)[0][0], "1337");

  EXPECT_STREQ((*elem)[1].name, "prop1");
  EXPECT_EQ((*elem)[1].size(), 1);
  EXPECT_STREQ((*elem)[1][0], "hello");

  EXPECT_STREQ((*elem)[2].name, "prop2");
  EXPECT_EQ((*elem)[2].size(), 1);
  EXPECT_STREQ((*elem)[2][0], "world");
}

void test_parse_nested() {
  std::string confstr =
      R"(
        outer  {
          inner {
            prop: 1337;
          }
        }
      )";

  PropertyListParser parser(confstr.c_str(), confstr.size());
  PropertyList plist;
  EXPECT(parser.parse(&plist) == true);

  EXPECT_EQ(plist.size(), 1);
  EXPECT_STREQ(plist[0].name, "outer");
  auto outer = plist[0].child.get();

  EXPECT_EQ(outer->size(), 1);
  EXPECT_STREQ((*outer)[0].name, "inner");
  auto inner = (*outer)[0].child.get();

  EXPECT_EQ(inner->size(), 1);
  EXPECT_STREQ((*inner)[0].name, "prop");
  EXPECT_EQ((*inner)[0].size(), 1);
  EXPECT_STREQ((*inner)[0][0], "1337");
}

void test_parse_value_list() {
  std::string confstr =
      R"(
        prop0: "123 456";
        prop1: 123 456;
      )";

  PropertyListParser parser(confstr.c_str(), confstr.size());
  PropertyList plist;
  if (!parser.parse(&plist)) {
    std::cerr << parser.get_error() << std::endl;
    std::exit(1);
  }

  EXPECT_EQ(plist.size(), 2);
  EXPECT_STREQ(plist[0].name, "prop0");
  EXPECT_STREQ(plist[0][0], "123 456");
  EXPECT_EQ(plist[0].size(), 1);
  EXPECT_STREQ(plist[0][0], "123 456");
  EXPECT_STREQ(plist[1].name, "prop1");
  EXPECT_EQ(plist[1].size(), 2);
  EXPECT_STREQ(plist[1][0], "123");
  EXPECT_STREQ(plist[1][1], "456");
}

void test_parse_value_comma() {
  std::string confstr =
      R"(
        prop0: 123,456;
        prop1: 123, 456;
        prop2: 123,  456;
        prop3: 123  , 456;
      )";

  PropertyListParser parser(confstr.c_str(), confstr.size());
  PropertyList plist;
  if (!parser.parse(&plist)) {
    std::cerr << parser.get_error() << std::endl;
    std::exit(1);
  }

  EXPECT_EQ(plist.size(), 4);
  EXPECT_EQ(plist[0].name, "prop0");
  EXPECT_EQ(plist[0].size(), 3);
  EXPECT_EQ(plist[0][0].is_literal, true);
  EXPECT_EQ(plist[0][1].is_literal, true);
  EXPECT_EQ(plist[0][2].is_literal, true);

  EXPECT_STREQ(plist[1].name, "prop1");
  EXPECT_EQ(plist[1].size(), 3);
  EXPECT_STREQ(plist[1][0], "123");
  EXPECT_STREQ(plist[1][1], ",");
  EXPECT_STREQ(plist[1][2], "456");
  EXPECT_STREQ(plist[2].name, "prop2");
  EXPECT_EQ(plist[2].size(), 3);
  EXPECT_STREQ(plist[2][0], "123");
  EXPECT_STREQ(plist[2][1], ",");
  EXPECT_STREQ(plist[2][2], "456");
  EXPECT_STREQ(plist[3].name, "prop3");
  EXPECT_EQ(plist[3].size(), 3);
  EXPECT_STREQ(plist[3][0], "123");
  EXPECT_STREQ(plist[3][1], ",");
  EXPECT_STREQ(plist[3][2], "456");
}

void test_parse_value_parens() {
  std::string confstr =
      R"(
        test: blah("1337");
      )";

  PropertyListParser parser(confstr.c_str(), confstr.size());
  PropertyList plist;
  if (!parser.parse(&plist)) {
    std::cerr << parser.get_error() << std::endl;
    std::exit(1);
  }

  EXPECT_EQ(plist.size(), 1);
  EXPECT_STREQ(plist[0].name, "test");
  EXPECT_EQ(plist[0].size(), 4);
  EXPECT_STREQ(plist[0][0], "blah");
  EXPECT_STREQ(plist[0][1], "(");
  EXPECT_STREQ(plist[0][2], "1337");
  EXPECT_STREQ(plist[0][3], ")");
  EXPECT_EQ(plist[0][0].is_literal, true);
  EXPECT_EQ(plist[0][1].is_literal, true);
  EXPECT_EQ(plist[0][2].is_literal, false);
  EXPECT_EQ(plist[0][3].is_literal, true);
}

int main() {
  test_parse_simple();
  test_parse_element();
  test_parse_nested();
  test_parse_value_list();
  test_parse_value_comma();
  test_parse_value_parens();
  return EXIT_SUCCESS;
}


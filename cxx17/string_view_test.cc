#include <gtest/gtest.h>

// https://en.cppreference.com/w/cpp/utility/functional/function
#include <string>
#include <string_view>
#include <iostream>

void print_address(const char* data) {
  std::cout << data << ": " << reinterpret_cast<uintptr_t>(data) << std::endl;
}

size_t length(const std::string& s) {
  print_address(s.data());
  return s.size();
}

size_t length_view(const std::string_view& s) {
  print_address(s.data());
  return s.size();
}

TEST(CxxTest, StringViewBasic) {
  // string_view is immutable
  std::string s = "Hello";
  print_address(s.data());
  s[0] = 'h';
  print_address(s.data());

  std::string_view sv = s;
  print_address(s.data());
  print_address(sv.data());

  const char* input = "Hello, World!";
  print_address(input);
  // string as function parameter
  length(input);
  // string_view as function parameter
  length_view(input);
}

TEST(CxxTest, StringViewUndefinedBehavior) {
  // undefined behavior
  std::string s = "abcde";
  std::string_view sv = s;
  print_address(s.data());
  print_address(sv.data());
  s[0] = 'A';
  print_address(s.data());
  print_address(sv.data());

  s = "1234567";
  print_address(s.data());
  print_address(sv.data());
  s = "a";
  print_address(s.data());
  print_address(sv.data());
}
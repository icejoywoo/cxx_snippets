#include <gtest/gtest.h>
#include <iostream>
#include <string>

#include <boost/type_index.hpp>

// 因为 TD 本身没有实现，所以编译器会报错，报错信息中会带有类型
template<typename T>
class TD; // TD is short for type displayer

// boost type_index
template<typename T>
void f(const T& param) {
  std::cout << "T = "
            << boost::typeindex::type_id_with_cvr<T>().pretty_name()
            << std::endl;

  std::cout << "param = "
            << boost::typeindex::type_id_with_cvr<decltype(param)>().pretty_name()
            << std::endl;
}

TEST(TemplateTest, showType) {
  auto x = 10;
  const auto& y = 10;

  // 最可靠的方法，通过编译器报错来获取实际类型
//  TD<decltype(x)> td1;
  // error: implicit instantiation of undefined template 'TD<int>'
  //  TD<decltype(x)> td1;
//  TD<decltype(y)> td2;
  // implicit instantiation of undefined template 'TD<const int &>'
  //  TD<decltype(y)> td2;

  // typeid name 不可靠
  std::cout << typeid(x).name() << std::endl; // i in mac
  std::cout << typeid(y).name() << std::endl; // i in mac

  f(x);
  f(y);

  std::vector<int> a(1, 0);
  const auto b = a;
  if (!b.empty()) {
    f(&b[0]);
  }
}
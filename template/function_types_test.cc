#include <gtest/gtest.h>
#include <iostream>
#include <string>
#include <utility>

// Checks if a class C provides a method which returns TRet and takes TArgs as
// parameters. TResolver is a struct created using DECLARE_METHOD_RESOLVER,
// which contains the name of the method.
template <typename C, class TResolver, typename TRet, typename... TArgs>
struct has_method {
 private:
 // 使用函数返回类型后置声明函数
  template <typename T>
  static constexpr auto check(T*) -> typename std::is_same<
      decltype(std::declval<TResolver>().template resolve<T>(
          std::declval<TArgs>()...)),
      TRet>::type {
    return {};
  }

  template <typename>
  static constexpr std::false_type check(...) {
    return std::false_type();
  }

  using type = decltype(check<C>(nullptr));

 public:
  static constexpr bool value = type::value;
};

// Declares a method resolver to be used with has_method.
#define DECLARE_METHOD_RESOLVER(Name, MethodName)              \
  struct Name {                                                \
    template <class __T, typename... __TArgs>                  \
    constexpr auto resolve(__TArgs&&... args) const            \
        -> decltype(std::declval<__T>().MethodName(args...)) { \
      return {};                                               \
    }                                                          \
  };

namespace detail {
// dummy resolver
template <typename T>
struct resolver {
  using in_type = T;
  using null_free_in_type = in_type;
  using out_type = T;
};
} // detail

struct Exec {
  template <typename T>
  using resolver = typename detail::template resolver<T>;
};

template <typename Fun, typename Exec, typename TReturn, typename... TArgs>
class UDFHolder {
  Fun instance_;
public:
  template <typename T>
  using exec_resolver = typename Exec::template resolver<T>;

  using exec_return_type = typename exec_resolver<TReturn>::out_type;
  template <typename T>
  using exec_arg_type = typename exec_resolver<T>::in_type;
  using exec_arg_types = std::tuple<typename exec_resolver<TArgs>::in_type...>;
  UDFHolder() {
    // std::cout << typeid(exec_return_type).name() << std::endl;
  }

  DECLARE_METHOD_RESOLVER(call_method_resolver, call);
  static constexpr bool udf_has_call_return_bool = has_method<
      Fun,
      call_method_resolver,
      bool,
      exec_return_type,
      const exec_arg_type<TArgs>&...>::value;
  
  inline bool callImpl(
      typename Exec::template resolver<TReturn>::out_type& out,
      const typename Exec::template resolver<TArgs>::in_type&... args) {
    if constexpr (udf_has_call_return_bool) {
      std::cout << "return_bool" << std::endl;
      return instance_.call(out, args...);
    } else {
      std::cout << "return_void" << std::endl;
      instance_.call(out, args...);
      return true;
    }
  }
};

struct TestVoidCall {
  void call(std::string& b) {};
};

struct TestBoolCall {
  bool call(int32_t& b) {
    return false;
  };
};

struct TestBoolCall3 {
  bool call(int32_t& b, int, std::string, bool) {
    return false;
  };
};

TEST(TemplateTest, HasCallReturnBoolTest) {
  {
    using TestVoidCall_holder = UDFHolder<TestVoidCall, Exec, std::string>;
    ASSERT_FALSE(TestVoidCall_holder::udf_has_call_return_bool);
    TestVoidCall_holder a;
    std::string s;
    a.callImpl(s);
  }

  {
    using TestBoolCall_holder = UDFHolder<TestBoolCall, Exec, int32_t>;
    ASSERT_TRUE(TestBoolCall_holder::udf_has_call_return_bool);
    TestBoolCall_holder a;
    int32_t i;
    a.callImpl(i);
  }

  {
    using TestBoolCall_holder = UDFHolder<TestBoolCall3, Exec, int32_t, int, std::string, bool>;
    ASSERT_TRUE(TestBoolCall_holder::udf_has_call_return_bool);
    TestBoolCall_holder a;
    int32_t i;
    a.callImpl(i, 0, "", false);
  }
}
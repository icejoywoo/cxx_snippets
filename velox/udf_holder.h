#pragma once

#include<iostream>
#include<optional>
#include<tuple>
#include<utility>

/// file: UdfTypeResolver.h
namespace detail {
// dummy resolver
    template <typename T>
    struct resolver {
        using in_type = T;
        using null_free_in_type = in_type;
        using out_type = T&;
    };
} // namespace detail

struct VectorExec {
    template <typename T>
    using resolver = typename detail::template resolver<T>;
};

/// file: Metaprogramming.h
namespace util {
// Checks if a class C provides a method which returns TRet and takes TArgs as
// parameters. TResolver is a struct created using DECLARE_METHOD_RESOLVER,
// which contains the name of the method.
template <typename C, class TResolver, typename TRet, typename... TArgs>
struct has_method {
private:
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
} // namespace util


// Declares a method resolver to be used with has_method.
#define DECLARE_METHOD_RESOLVER(Name, MethodName)              \
  struct Name {                                                \
    template <class __T, typename... __TArgs>                  \
    constexpr auto resolve(__TArgs&&... args) const            \
        -> decltype(std::declval<__T>().MethodName(args...)) { \
      return {};                                               \
    }                                                          \
  };

/// file: SimpleFunctionMetadata.h
// wraps a UDF object to provide the inheritance
// this is basically just boilerplate-avoidance
template <typename Fun, typename Exec, typename TReturn, typename... TArgs>
class UDFHolder final {
    Fun instance_;
public:
    template <typename T>
    using exec_resolver = typename Exec::template resolver<T>;

    using exec_return_type = typename exec_resolver<TReturn>::out_type;
    using optional_exec_return_type = std::optional<exec_return_type>;

    template <typename T>
    using exec_arg_type = typename exec_resolver<T>::in_type;
    using exec_arg_types = std::tuple<typename exec_resolver<TArgs>::in_type...>;

    DECLARE_METHOD_RESOLVER(call_method_resolver, call);

    // call():
    static constexpr bool udf_has_call_return_bool = util::has_method<
            Fun,
            call_method_resolver,
            bool, // call() return value
            exec_return_type, // udf return value
            const exec_arg_type<TArgs>&...>::value; // TArgs udf args

    // Helper functions to handle void vs bool return type.

    inline bool callImpl(
            typename Exec::template resolver<TReturn>::out_type& out,
            const typename Exec::template resolver<TArgs>::in_type&... args) {
        if constexpr (udf_has_call_return_bool) {
            std::cout << "return bool" << std::endl;
            return instance_.call(out, args...);
        } else {
            std::cout << "return void" << std::endl;
            instance_.call(out, args...);
            return true;
        }
    }
};

struct TestVoidCall {
    void call(std::string& out, int32_t a, bool b) {};
};

struct TestBoolCall {
    bool call(int32_t&) {
        return false;
    };
};
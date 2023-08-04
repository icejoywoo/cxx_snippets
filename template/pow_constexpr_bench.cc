// https://wendeng.github.io/2019/06/30/c++%E5%9F%BA%E7%A1%80/C++%E5%9F%BA%E7%A1%80%E4%B9%8B%E5%85%83%E7%BC%96%E7%A8%8B%E4%B8%8Econstexpr/

#include <benchmark/benchmark.h>

#include <iostream>

int pow(int a, int b)
{
    if (b == 0) return 1;
    if (b == 1) return a;
    return a * pow(a, b - 1);
}

template <int A, int B> 
struct ConstPow
{
    constexpr static int result = A * ConstPow<A, B - 1>::result;
};

template <int A> struct ConstPow<A, 1> { constexpr static int result = A; };
template <int A> struct ConstPow<A, 0> { constexpr static int result = 1; };

// dot
// 递归实现
template <class T> T dot_re(int dim, T *a, T *b)
{
    if (dim == 1) return (*a) * (*b);
    return (*a) * (*b) + dot_re(dim - 1, a + 1, b + 1);
}

// 迭代实现
template <class T> T dot(int dim, T *a, T *b)
{
    int result = T();
    for (int i = 0; i < dim; i++)
        result += a[i] * b[i];
    return result;
}

// 模板实现
template <int DIM, class T> 
class __Dot
{
public:
    static T result(T *a, T *b)
    {
        return (*a) * (*b) + __Dot<DIM - 1, T>::result(a + 1, b + 1);
    }
};

template <class T> 
class __Dot<1, T>
{
public:
    static T result(T *a, T *b) { return (*a) * (*b); }
};

template <int DIM, class T>
T dot_meta(T *a, T *b)
{
    return __Dot<DIM, T>::result(a, b);
}
// volatile 的意义：https://www.zhihu.com/question/329656785
template<typename T>
void black_hole(T t) {
  // volatile并不保证未使用的变量不被抛弃，本意是提醒编译器这个变量可能会在代码之外被改变
  volatile T a = t;
}

static void BM_pow(benchmark::State& state) {
  // Perform setup here
  for (auto _ : state) {
    // This code gets timed
    black_hole(pow(2, 4));
  }
}

static void BM_pow_template(benchmark::State& state) {
  // Perform setup here
  for (auto _ : state) {
    // This code gets timed
    black_hole(ConstPow<2, 4>::result);
  }
}

// dot benchmark
std::vector<int> a = {1, 2, 3, 4}, b = {5, 6, 7, 8};

static void BM_dot(benchmark::State& state) {
  // Perform setup here
  for (auto _ : state) {
    // This code gets timed
    black_hole(dot(4, a.data(), b.data()));
  }
}

static void BM_dot_re(benchmark::State& state) {
  // Perform setup here
  for (auto _ : state) {
    // This code gets timed
    black_hole(dot_re(4, a.data(), b.data()));
  }
}

static void BM_dot_meta(benchmark::State& state) {
  // Perform setup here
  for (auto _ : state) {
    // This code gets timed
    black_hole(dot_meta<4>(a.data(), b.data()));
  }
}

// Register the function as a benchmark
// ->RangeMultiplier(2)->Range(8, 8<<10)
BENCHMARK(BM_pow);
BENCHMARK(BM_pow_template);

BENCHMARK(BM_dot);
BENCHMARK(BM_dot_re);
BENCHMARK(BM_dot_meta);
// Run the benchmark
BENCHMARK_MAIN();
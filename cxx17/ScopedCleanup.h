#pragma once

#include <utility>

/// copied from "kudu/gutil/macros.h"

// Macro that allows definition of a variable appended with the current line
// number in the source file. Typically for use by other macros to allow the
// user to declare multiple variables with the same "base" name inside the same
// lexical block.
#define VARNAME_LINENUM(varname) VARNAME_LINENUM_INTERNAL(varname ## _L, __LINE__)
#define VARNAME_LINENUM_INTERNAL(v, line) VARNAME_LINENUM_INTERNAL2(v, line)
#define VARNAME_LINENUM_INTERNAL2(v, line) v ## line

/// https://github.com/apache/kudu/blob/master/src/kudu/util/scoped_cleanup.h

// Run the given function body (which is typically a block of code surrounded by
// curly-braces) when the current scope exits.
//
// Example:
//   int fd = open(...);
//   SCOPED_CLEANUP({ close(fd); });
//
// NOTE: in the case that you want to cancel the cleanup, use the more verbose
// (non-macro) form below.
#define SCOPED_CLEANUP(func_body) \
  auto VARNAME_LINENUM(scoped_cleanup) = ::common::MakeScopedCleanup([&] { func_body })

namespace common {

// A scoped object which runs a cleanup function when going out of scope. Can
// be used for scoped resource cleanup.
//
// Use 'MakeScopedCleanup()' below to instantiate.
template<typename F>
class ScopedCleanup {
 public:
  explicit ScopedCleanup(F f)
      : cancelled_(false),
        f_(std::move(f)) {
  }
  ~ScopedCleanup() {
    if (!cancelled_) {
      f_();
    }
  }
  void cancel() { cancelled_ = true; }

 private:
  bool cancelled_;
  F f_;
};

// Creates a new scoped cleanup instance with the provided function.
template<typename F>
ScopedCleanup<F> MakeScopedCleanup(F f) {
  return ScopedCleanup<F>(f);
}

} // namespace common
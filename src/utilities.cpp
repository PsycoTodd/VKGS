#include "utilities.h"
#include "precomp.h"

namespace todd {
  bool streq(const gsl::czstring& left, const gsl::czstring& right)
  {
    return std::strcmp(left, right) == 0;
  }
};

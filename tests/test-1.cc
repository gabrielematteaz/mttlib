#include <print>

#include "nothrow_string.h"

int main() {
  auto string = mttlib::nothrow_string::construct("ciao io sono");

  if (string.has_value() == false) {
    return false;
  }

  std::print("{}", mttlib::hash < mttlib::nothrow_string > () (string.value()));
}
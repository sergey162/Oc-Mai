#include <cstdint>
#include <utility>
#include <algorithm>

extern "C" {

std::int32_t FastGCF(std::int32_t first, std::int32_t second) {
  while (second != 0) {
    first = std::exchange(second, first % second);
  }
  return first;
}


std::int32_t BadGCF(std::int32_t first, std::int32_t second) {
  for (std::int32_t min = std::min(first, second); min > 0; --min) {
    if (first % min == 0 && second % min == 0) {
      return min;
    }
  }
  return 1;
}

}

#include <cstdint>
#include <cmath>
#include <vector>

extern "C" {

std::int32_t FastCountSimpleNumbers(std::int32_t first, std::int32_t second) {
  std::vector numbers(second + 1, true);
  numbers[0] = numbers[1] = false;
  for (std::int32_t iter = 2; iter <= std::sqrt(second); ++iter) {
    if (numbers[iter]) {
      for (std::int32_t count = iter * iter; count <= second; count += iter) {
        numbers[count] = false;
      }
    }
  }
  std::int32_t count = 0;
  for (std::int32_t iter = first; iter <= second; ++iter) {
    if (numbers[iter]) {
      ++count;
    }
  }
  return count;
}

std::int32_t BadCountSimpleNumbers(std::int32_t first, std::int32_t second) {
  if (second < 2) {
    return 0;
  } else if (first < 2) {
    first = 2;
  }
  std::int32_t count = 0;
  for (std::int32_t number = first; number <= second; ++number) {
    bool is_simple = true;
    for (std::int32_t iter = 2; iter <= std::sqrt(number); ++iter) {
      if (number % iter == 0) {
        is_simple = false;
        break;
      }
    }
    if (is_simple) {
      ++count;
    }
  }
  return count;
}

}

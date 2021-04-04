#ifndef FF_UTILS_MATH_H_
#define FF_UTILS_MATH_H_

namespace math {

inline constexpr long long int ConstexprPow(int n, int p) {
  long long int result = n;
  for (int i = 0; i < p; i++) result = result*n;
  return result;
}

}; // namespace math

#endif


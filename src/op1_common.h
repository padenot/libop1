#ifndef OP1_COMMON_H
#define OP1_COMMON_H

extern bool g_logging_enabled;
#include <iterator>

#define OP1_MACRO_BEGIN do {
#define OP1_MACRO_END } while (0)

#define LOG(...)                                       \
    OP1_MACRO_BEGIN                                    \
    if (g_logging_enabled) {                           \
      fprintf(stderr, "%s:%d: ", __FILE__, __LINE__);  \
      fprintf(stderr, __VA_ARGS__);                    \
    }                                                  \
    OP1_MACRO_END

#define FATAL(str)                       \
  OP1_MACRO_BEGIN                        \
  fprintf(stderr, "Fatal: %s\n", (str)); \
  exit(1);                               \
  OP1_MACRO_END

#define WARN(str)                          \
  OP1_MACRO_BEGIN                          \
  fprintf(stderr, "Warning: %s\n", (str)); \
  OP1_MACRO_END

#define ENSURE_VALID(ptr)         \
  OP1_MACRO_BEGIN                 \
    if (!ptr) {                   \
      return OP1_ARGUMENT_ERROR;  \
    }                             \
  OP1_MACRO_END

template<typename T>
void PodZero(T blob)
{
  memset(&blob, 0, sizeof(T));
}


template<typename T, size_t S>
void ArrayCopy(std::array<T, S> lhs, T rhs[S])
{
  for (size_t i = 0; i < S; i++) {
    lhs[i] = rhs[i];
  }
}

#endif // OP1_COMMON_H

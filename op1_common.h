#ifndef OP1_COMMON_H
#define OP1_COMMON_H

extern bool g_logging_enabled;

#define LOG(...) do {                                  \
    if (g_logging_enabled) {                           \
      fprintf(stderr, "%s:%d: ", __FILE__, __LINE__);  \
      fprintf(stderr, __VA_ARGS__);                    \
    }                                                  \
  } while(0)

#define FATAL(str) \
  fprintf(stderr, "Fatal: %s\n", (str)); \
  exit(1)

#define WARN(str) \
  fprintf(stderr, "Warning: %s\n", (str));

template<typename T>
void PodZero(T blob)
{
  memset(&blob, 0, sizeof(T));
}


#endif // OP1_COMMON_H

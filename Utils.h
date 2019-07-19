#ifndef UTILS_H
#define UTILS_H
int StringEquals(const char *s1, const char *s2) {
  while (*s1 == *s2++)
    if (*s1++ == 0)
      return 1;
  return 0;
}
#endif
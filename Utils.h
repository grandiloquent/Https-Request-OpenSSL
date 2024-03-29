#ifndef UTILS_H
#define UTILS_H

#include "string.h"

int StringEquals(const char* s1, const char* s2)
{
    while (*s1 == *s2++)
        if (*s1++ == 0)
            return 1;
    return 0;
}
int IsDigit(const char* s)
{
    if (s != 0)
    {
        while (*s >= '0' && *s <= '9')
            if (*++s == 0)
                return 1;
    }
    return 0;
}
inline bool EndsWith(const char* s1, const char* s2)
{
    size_t s1_length = strlen(s1);
    size_t s2_length = strlen(s2);
    if (s2_length > s1_length)
    {
        return false;
    }
    return memcmp(s1 + (s1_length - s2_length), s2, s2_length) == 0;
}

int StringRangeEquals(const char* s1, const char* s2, size_t n)
{

    if (n == 0)
        return (0);
    do
    {
        if (*s1 != *s2++)
            return (0);
        if (*s1++ == 0)
            break;
    } while (--n != 0);
    return (1);
}

char* StringConcat(char* s, const char* append)
{

    char* save = s;
    for (; *s; ++s)
        ;
    while ((*s++ = *append++) != '\0')
        ;
    return (save);
}
char* StringCopy(char* to, const char* from)
{
    char* save = to;

    for (; (*to = *from) != '\0'; ++from, ++to)
        ;
    return (save);
}

std::string FindObj(std::string s1, const char* s2)
{

    auto idx = s1.find(s2);
    if (idx == -1)
        return "";

    auto len = idx + strlen(s2);
    if (len == s1.length())
        return "";

    auto start = s1.find("\"", len);

    if (start == s1.length())
        return "";
    start++;

    auto end = s1.find("\"", start);
    if (end == -1)
        return "";

    while (end - 1 >= 0 && s1.at(end - 1) == '\\')
    {

        end = s1.find("\"", end + 1);
    }

    if (end > start)
        return s1.substr(start, end - start);
    return "";
}
char* StringString(const char* s, const char* find)
{
    char c, sc;
    size_t len;

    if ((c = *find++) != 0)
    {
        len = strlen(find);
        do
        {
            do
            {
                if ((sc = *s++) == 0)
                    return (NULL);
            } while (sc != c);
        } while (StringRangeEquals(s, find, len));
        s--;
    }
    return ((char*)s);
}
#endif
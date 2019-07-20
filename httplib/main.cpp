#include <iostream>
#include <map>
#include <cstring>

void Map(std::map<const char*, const char*>& map)
{
    map["User-Agent"] = "Browser";
    for (auto ite = map.begin(); ite != map.end(); ++ite)
    {
        std::cout << ite->first << std::endl;
    }
}

std::string FindObj(std::string s1, const char* s2)
{

    auto idx = s1.find(s2);
    if (idx == -1)
        return std::string();

    auto len = idx + strlen(s2);
    if (len == s1.length())
        return std::string();

    auto start = s1.find("\"", len);

    if (start == s1.length())
        return std::string();
    start++;

    auto end = s1.find("\"", start);
    if (end == -1)
        return std::string();

    while (end - 1 >= 0 && s1.at(end - 1) == '\\')
    {

        end = s1.find("\"", end + 1);
    }

    if (end > start)
        return s1.substr(start, end - start);
    return std::string();
}
int main(int argc, char const* argv[])
{
    std::cout << IsDigit(" ") << std::endl;
    std::cout << IsDigit(nullptr) << std::endl;

    std::cout << IsDigit(" 213232133213") << std::endl;
    std::cout << IsDigit("213232133213") << std::endl;
    std::string s = FindObj("\"key\":\"\\\"23\"", "\"key\"");
    std::cout << s << std::endl;
    return 0;
}

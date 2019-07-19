#include "detail.h"
#include <iostream>

int main(int argc, char const* argv[])
{
    int i = 0;
    std::cout << httplib::detail::is_hex('a', i) << " " << i << std::endl;
    return 0;
}
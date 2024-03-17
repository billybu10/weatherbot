#pragma once
#include <string>
#include <sstream>

//copied from here
//https://stackoverflow.com/questions/16605967/set-precision-of-stdto-string-when-converting-floating-point-values/16606128#16606128
//converts floats to a string with set number of  digits behind comma/dot
template <typename T>
std::string to_string_with_precision(const T a_value, const int n = 1)
{
    std::ostringstream out;
    out.precision(n);
    out << std::fixed << a_value;
    return std::move(out).str();
}

#include <stdexcept>
#include <cstring>
#include "stringext.h"

namespace string {

const char* WHITESPACE=" \n\r\t\f\v";

bool startswith(const std::string& text, const std::string& prefix) {
    if (text.length() >= prefix.length()) {
        return (0 == text.compare (0, prefix.length(), prefix));
    } else {
        return false;
    }
}

bool startswith(const std::string_view text, const std::string_view prefix) {
    if (text.length() >= prefix.length()) {
        return (0 == text.compare (0, prefix.length(), prefix));
    } else {
        return false;
    }
}

bool endswith(const std::string& text, const std::string& suffix) {
    if (text.length() >= suffix.length()) {
        return (0 == text.compare (text.length() - suffix.length(), suffix.length(), suffix));
    } else {
        return false;
    }
}

bool endswith(const std::string_view text, const std::string_view suffix) {
    if (text.length() >= suffix.length()) {
        return (0 == text.compare (text.length() - suffix.length(), suffix.length(), suffix));
    } else {
        return false;
    }
}

std::string lstrip(const std::string &text, const char* erase_chars)
{
    std::size_t i_start = text.find_first_not_of(erase_chars);
    return (i_start == std::string::npos) ? "" : text.substr(i_start);
}

std::string rstrip(const std::string &text, const char* erase_chars)
{
    std::size_t i_end = text.find_last_not_of(erase_chars);
    return (i_end == std::string::npos) ? "" : text.substr(0, i_end + 1);
}

std::string strip(const std::string &text, const char* erase_chars)
{
    std::size_t i_start = text.find_first_not_of(erase_chars);
    if (i_start != std::string::npos) {
        std::size_t i_end = text.find_last_not_of(erase_chars);
        return text.substr(i_start, i_end - i_start + 1);
    }
    return "";
}

std::vector<std::string> split(const std::string &text, 
    const std::string_view sep, std::size_t maxsplit)
{
    if (sep.length() == 0) {
        throw std::invalid_argument("empty separator");
    }
    
    std::size_t i_start = 0;
    std::size_t i_end = text.find(sep, i_start);
    size_t n_elements=1;
    while ((i_end < std::string::npos) && (maxsplit-- > 0)) {
        n_elements++;
        i_start = i_end + sep.length(); 
        i_end = text.find(sep, i_start);
    }

    std::vector<std::string> result;
    result.reserve(n_elements);
    i_start = 0;
    i_end = text.find(sep, i_start);

    while (--n_elements > 0) {
        result.push_back( text.substr(i_start, i_end - i_start) );
        i_start = i_end + sep.length(); 
        i_end = text.find(sep, i_start);
    }
    result.push_back( text.substr(i_start, std::string::npos) );
    return result;
}

std::vector<std::string_view> split(const std::string_view text, 
    const std::string_view sep, std::size_t maxsplit)
{
    if (sep.length() == 0) {
        throw std::invalid_argument("empty separator");
    }
    
    std::size_t i_start = 0;
    std::size_t i_end = text.find(sep, i_start);
    size_t n_elements=1;
    while ((i_end < std::string::npos) && (maxsplit-- > 0)) {
        n_elements++;
        i_start = i_end + sep.length(); 
        i_end = text.find(sep, i_start);
    }

    std::vector<std::string_view> result;
    result.reserve(n_elements);
    i_start = 0;
    i_end = text.find(sep, i_start);

    while (--n_elements > 0) {
        result.push_back( text.substr(i_start, i_end - i_start) );
        i_start = i_end + sep.length(); 
        i_end = text.find(sep, i_start);
    }
    result.push_back( text.substr(i_start, std::string::npos) );
    return result;
}


std::vector<std::string> split(const std::string &text, std::size_t maxsplit)
{
    std::vector<std::string> result;
    std::size_t i_start = text.find_first_not_of(WHITESPACE);
    std::size_t i_end = text.find_first_of(WHITESPACE, i_start+1);

    while ((i_start < i_end) && (i_end < std::string::npos) && (maxsplit > 0)) {
        maxsplit--;
        result.push_back( text.substr(i_start, i_end - i_start) );
        i_start = text.find_first_not_of(WHITESPACE, i_end+1);
        i_end = text.find_first_of(WHITESPACE, i_start+1);
    }

    if (i_start < std::string::npos) {
        if (maxsplit > 0) {
            result.push_back(text.substr(i_start, i_end - i_start));
        } else {
            result.push_back(text.substr(i_start));
        }
    }
    return result;
}

std::vector<std::string_view> split(const std::string_view text, std::size_t maxsplit)
{
    std::vector<std::string_view> result;
    std::size_t i_start = text.find_first_not_of(WHITESPACE);
    std::size_t i_end = text.find_first_of(WHITESPACE, i_start+1);

    while ((i_start < i_end) && (i_end < std::string::npos) && (maxsplit > 0)) {
        maxsplit--;
        result.push_back( text.substr(i_start, i_end - i_start) );
        i_start = text.find_first_not_of(WHITESPACE, i_end+1);
        i_end = text.find_first_of(WHITESPACE, i_start+1);
    }

    if (i_start < std::string::npos) {
        if (maxsplit > 0) {
            result.push_back(text.substr(i_start, i_end - i_start));
        } else {
            result.push_back(text.substr(i_start));
        }
    }
    return result;
}

std::vector<std::string> rsplit(const std::string &text, 
    const std::string_view sep, std::size_t maxsplit)
{
    if (sep.length() == 0) {
        throw std::invalid_argument("empty separator");
    }
    
    std::size_t i_end = text.length()-1;
    std::size_t i_start = text.rfind(sep, std::string::npos);

    size_t n_elements = 1;
    while ((i_start < std::string::npos) && (maxsplit-- > 0)) {
        n_elements++;
        i_end = i_start-1;
        i_start = text.rfind(sep, i_end);
    }

    std::vector<std::string> result;
    result.reserve(n_elements);
    i_end++;
    result.push_back( text.substr(0, i_end) );
    i_start = i_end + sep.length();
    i_end = text.find(sep, i_start);

    while (n_elements-- > 1) {
        result.push_back( text.substr(i_start, i_end - i_start) );
        i_start = i_end + sep.length(); 
        i_end = text.find(sep, i_start);
    }
    return result;
}

std::vector<std::string_view> rsplit(const std::string_view text, 
    const std::string_view sep, std::size_t maxsplit)
{
    if (sep.length() == 0) {
        throw std::invalid_argument("empty separator");
    }
    
    std::size_t i_end = text.length()-1;
    std::size_t i_start = text.rfind(sep, std::string::npos);

    size_t n_elements = 1;
    while ((i_start < std::string::npos) && (maxsplit-- > 0)) {
        n_elements++;
        i_end = i_start-1;
        i_start = text.rfind(sep, i_end);
    }

    std::vector<std::string_view> result;
    result.reserve(n_elements);
    i_end++;
    result.push_back( text.substr(0, i_end) );
    i_start = i_end + sep.length();
    i_end = text.find(sep, i_start);

    while (--n_elements > 0) {
        result.push_back( text.substr(i_start, i_end - i_start) );
        i_start = i_end + sep.length(); 
        i_end = text.find(sep, i_start);
    }
    return result;
}

}
#ifndef __STRINGEXT_H_INCLUDED
#define __STRINGEXT_H_INCLUDED

#include <string>
#include <string_view>
#include <memory>
#include <stdexcept>
#include <vector>
#include <algorithm>

namespace string {
    extern const char *WHITESPACE;

    extern bool startswith(const std::string& text, const std::string& prefix);
    extern bool startswith(const std::string_view text, const std::string_view prefix);
    extern bool endswith(const std::string& text, const std::string& prefix);
    extern bool endswith(const std::string_view text, const std::string_view prefix);
    extern std::string lstrip(const std::string &text, const char* erase_chars=WHITESPACE);
    extern std::string rstrip(const std::string &text, const char* erase_chars=WHITESPACE);
    extern std::string strip(const std::string &text, const char* erase_chars=WHITESPACE);
    extern std::vector<std::string> split(const std::string &text, 
        const std::string_view sep, std::size_t maxsplit=-1);
    // Split based on the whitspices
    extern std::vector<std::string> split(const std::string &text, std::size_t maxsplit=-1);
    extern std::vector<std::string_view> split(const std::string_view text,
        std::size_t maxsplit);
    extern std::vector<std::string_view> split(const std::string_view text, 
        const std::string_view sep, std::size_t maxsplit=-1);
    extern std::vector<std::string> rsplit(const std::string &text, 
        const std::string_view sep, std::size_t maxsplit=-1);
    extern std::vector<std::string_view> rsplit(const std::string_view text, 
        const std::string_view sep, std::size_t maxsplit=-1);

    // Returns copy of original string with lowercase
    inline std::string lower(std::string text) {
        std::transform(text.begin(), text.end(), text.begin(), 
            [](unsigned char c){ return std::tolower(c); }
        );
        return text;
    }

    inline std::string lower(std::string_view text) {
        std::string result;
        result.reserve(text.size());

        std::transform(text.begin(), text.end(), std::back_insert_iterator(result), 
            [](unsigned char c){ return std::tolower(c); }
        );
        return result;
    }

    // https://stackoverflow.com/questions/2342162/stdstring-formatting-like-sprintf
    template<typename ... Args>
    std::string format( const std::string& format, Args ... args )
    {
        int size_s = std::snprintf( nullptr, 0, format.c_str(), args ... ) + 1; // Extra space for '\0'
        if( size_s <= 0 ){ 
            throw std::runtime_error( "Error during formatting." ); 
        }
        auto size = static_cast<size_t>( size_s );
        auto buf = std::make_unique<char[]>( size );
        std::snprintf( buf.get(), size, format.c_str(), args ... );
        return std::string( buf.get(), buf.get() + size - 1 ); // We don't want the '\0' inside
    }

}

#endif
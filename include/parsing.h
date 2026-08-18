#pragma once
#include <iterator>
#include <cstdint>
#include <cstring>
#include <charconv>
#include <array>


namespace CustomParsing {
    constexpr std::uint64_t powtable[] = {1, 10, 100, 1'000, 10'000};
    template <std::contiguous_iterator T>
    void int_from_float_chars(T begin, T end, std::uint64_t& val) {
       auto decimal = static_cast<const char*>(std::memchr(begin, '.', static_cast<std::size_t>(end - begin)));

       std::uint64_t whole = 0;
       for (const char* i{begin}; i != decimal; i++) {
           whole = whole * 10 + static_cast<std::uint64_t>(*i - '0');
       }

       std::uint64_t frac{};
       for (const char* i{decimal + 1}; i != end; i++) {
           frac = frac * 10 + static_cast<std::uint64_t>(*i - '0');
       }
       auto frac_digits = static_cast<std::size_t>(end - (decimal + 1));

       val = whole * powtable[4] + frac * powtable[4 - frac_digits];
    } 

    constexpr std::uint64_t Splat8Char0 =  0x3030303030303030;
    constexpr std::uint64_t mask = 0x000000FF000000FF;
    constexpr std::uint64_t mul1 = 100 + (1'000'000ULL << 32); // = 0x000F424000000064
    constexpr std::uint64_t mul2 = 1 + (10'000ULL << 32); // = 0x0000271000000001

    template <typename T>
    auto swar_eight_digits(const char* chars) -> T {
       uint64_t val;
       memcpy(&val, chars, 8);
       val -= Splat8Char0;
       val = (val * 10) + (val >> 8);
       val = ((val & mask) * mul1 + ((val >> 16) & mask) * mul2) >> 32;
       return val;
    }

    template <typename T>
    inline auto left_pad_and_swar(std::string_view& sv) -> T {
        std::array<char, 8> t_arr{};
        std::memset(t_arr.data(), '0', 8);
        auto sz = sv.size();
        auto start = 8 - sz;
        std::memcpy(t_arr.data() + start, sv.data(), sz);
        
        return swar_eight_digits<T>(t_arr.data());  
    }

    inline std::uint64_t parse_timestamp_beginning(std::string_view& sv) {
        std::uint64_t since_epoch{0};
        auto ptr = sv.data();
        for (auto i{0uz}; i < 3; i++) {
            since_epoch = since_epoch * 10 + static_cast<std::uint64_t>(*(ptr + i) - '0');
        }
        return since_epoch;
    }
};

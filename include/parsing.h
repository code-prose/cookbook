#pragma once
#include <iterator>
#include <cstdint>
#include <cstring>


namespace CustomParsing {
   constexpr std::uint64_t powtable[] = {1, 10, 100, 1'000, 10'000};
   template <std::contiguous_iterator T>
   void int_from_float_chars(T begin, T end, std::uint64_t& val) {
       auto decimal = std::memchr(begin, '.', static_cast<std::size_t>(begin - end));
       std::uint64_t first_half = stoll(begin, decimal);
       std::uint64_t second_half = stoll(decimal + 1, end);
       first_half *= powtable[4 - static_cast<std::size_t>(end - decimal + 1)];
       val = first_half + second_half;
   } 
};

#pragma once
#include <iterator>
#include <cstdint>
#include <cstring>
#include <charconv>


namespace CustomParsing {
   constexpr std::uint64_t powtable[] = {1, 10, 100, 1'000, 10'000};
   template <std::contiguous_iterator T>
   void int_from_float_chars(T begin, T end, std::uint64_t& val) {
      auto decimal = static_cast<const char*>(std::memchr(begin, '.', static_cast<std::size_t>(end - begin)));

      std::uint64_t whole{};
      std::from_chars(begin, decimal, whole);

      std::uint64_t frac{};
      auto [ptr, ec] = std::from_chars(decimal + 1, end, frac);
      auto frac_digits = static_cast<std::size_t>(ptr - (decimal + 1));

      val = whole * powtable[4] + frac * powtable[4 - frac_digits];
   } 

   constexpr std::uint64_t Splat8Char0 =  0x3030303030303030;
   constexpr std::uint64_t mask = 0x000000FF000000FF;
   constexpr std::uint64_t mul1 = 100 + (1'000'000ULL << 32); // = 0x000F424000000064
   constexpr std::uint64_t mul2 = 1 + (10'000ULL << 32); // = 0x0000271000000001

   inline std::uint64_t swar_eight_digits(const char* chars) {
      uint64_t val;
      memcpy(&val, chars, 8);
      val -= Splat8Char0;
      val = (val * 10) + (val >> 8);
      val = ((val & mask) * mul1 + ((val >> 16) & mask) * mul2) >> 32;
      return val;
   }
};

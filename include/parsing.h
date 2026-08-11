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
};

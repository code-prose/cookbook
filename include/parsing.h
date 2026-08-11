#pragma once
#include <iterator>
#include <cstdint>
#include <cstring>


namespace CustomParsing {
   template <std::contiguous_iterator T>
   void int_from_float_chars(T begin, T end, std::uint64_t& val) {
       auto decimal = std::memchr(begin, '.', static_cast<std::size_t>(begin - end));

   } 
};

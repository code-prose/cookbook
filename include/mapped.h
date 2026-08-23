#pragma once
#include <string>
#include <exception>
#include <iostream>
#include <string_view>
#include <cstring>
#include <array>

#include <immintrin.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

struct MappedFile {
    MappedFile(const std::string& path) : path_{path} {}

    void open() {
        fd_ = ::open(path_.c_str(), O_RDONLY);
        if (fd_ == -1) {
            std::cout << "Failed to open fd\n";
            std::terminate();
        }

        struct stat file_stats{};
        fstat(fd_, &file_stats);
        size_ = file_stats.st_size;


        long page_size = ::sysconf(_SC_PAGESIZE);
        std::size_t aligned_size = (size_ + page_size - 1) & ~(page_size - 1);
        guard_size_ = aligned_size + page_size;
        auto guarded = mmap(nullptr, guard_size_, PROT_READ, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        if (guarded == MAP_FAILED) {
            std::cout << "Failed to reserve guard space\n";
            std::terminate();
        }

        mapped_ = static_cast<const char*>(mmap(guarded, file_stats.st_size, PROT_READ, MAP_PRIVATE | MAP_FIXED, fd_, 0));
        if (mapped_ == MAP_FAILED) {
            std::cout << "Failed to map file\n";
            std::terminate();
        }
        madvise((void*)mapped_, file_stats.st_size, MADV_SEQUENTIAL);
        eof_ = mapped_ + size_;
        curr_ = mapped_;

    }

    void discard_headers() {
        const char* eol = static_cast<const char*>(std::memchr(curr_, '\n', 1000));
        curr_ = eol + 1;
    }

    // it is time for this to die... onto SIMD
    // trades ~60B, quotes ~74B
    static std::array<std::string_view, 10> split_sv(const std::string_view& sv) {
        constexpr auto NUM_COMMAS{9uz};
        std::array<std::string_view, 10> split{};
        const auto SPLATCOMMA = _mm512_set1_epi8(',');
        auto vectorized = _mm512_loadu_epi8(sv.data());
        auto mask = _mm512_cmpeq_epi8_mask(vectorized, SPLATCOMMA);
        auto next = _tzcnt_u64(mask);
        auto index{0uz};
        auto curr = sv.begin();
        auto count = 0;
        while (next < 64 && count != NUM_COMMAS) {
            auto comma = sv.data() + next;
            split[index++] = std::string_view{curr, static_cast<std::size_t>(comma - curr)}; 
            curr = comma + 1;
            mask &= mask - 1;
            next = _tzcnt_u64(mask);
        }
        // grab last
        if (count == NUM_COMMAS) {
            split[index] = std::string_view{curr, static_cast<std::size_t>(sv.end() - curr)};
        } else { // or grab remaining
            auto newer = curr;
            const char* next_comma = static_cast<const char*>(std::memchr(curr, ',', sv.end() - curr));
            while (next_comma) {
               split[index++] = std::string_view{curr, static_cast<std::size_t>(next_comma - curr)};
               curr = next_comma + 1;
               next_comma = static_cast<const char*>(std::memchr(curr, ',', sv.end() - curr));
            }
            split[index] = std::string_view{curr, static_cast<std::size_t>(sv.end() - curr)};
        }
        return split;
    }

    bool getline(std::string_view& sv) {
        if (curr_ >= eof_) {
            return false;
        }

        auto bytes_left = eof_ - curr_;
        const char* eol = static_cast<const char*>(std::memchr(curr_, '\n', bytes_left));

        std::size_t sz;
        const char* line_start = curr_;
        if (eol) {
            sz = eol - curr_;
            curr_ = eol + 1;
        } else {
            sz = bytes_left;
            curr_ = eof_;
        }

        sv = std::string_view{line_start, sz};
        return true;
    }

    std::size_t size() { return size_; }

    void close() {
        if (fd_ == -1) std::terminate();
        if (mapped_ == MAP_FAILED) std::terminate();

        munmap((void*)mapped_, guard_size_);
    }

    const char* eof_{};
    private:
        int fd_{};
        const char* mapped_{};
        const char* curr_{};
        std::string path_{};
        std::size_t size_{};
        std::size_t guard_size_{};
};

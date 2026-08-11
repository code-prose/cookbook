#pragma once
#include <string>
#include <exception>
#include <iostream>
#include <string_view>
#include <cstring>
#include <vector>

#include <sys/mman.h>
#include <fcntl.h>
#include <sys/stat.h>

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

        mapped_ = static_cast<const char*>(mmap(0, file_stats.st_size, PROT_READ, MAP_PRIVATE, fd_, 0));
        if (mapped_ == MAP_FAILED) {
            std::cout << "Failed to map file\n";
            std::terminate();
        }
        eof_ = mapped_ + size_;
        curr_ = mapped_;

    }

    void discard_headers() {
        const char* eol = static_cast<const char*>(std::memchr(curr_, '\n', 1000));
        curr_ = eol + 1;
    }

    static std::vector<std::string_view> split_sv(std::string_view& sv) {
        std::vector<std::string_view> vec{};
        const char* begin = sv.begin();
        const char* comma = static_cast<const char*>(std::memchr(begin, ',', sv.end() - begin));
        while(comma) {
           vec.push_back(std::string_view{begin, static_cast<std::size_t>(comma - begin)});
           begin = comma + 1;
           comma = static_cast<const char*>(std::memchr(begin, ',', sv.end() - begin));
        }
        vec.push_back(std::string_view{begin, static_cast<std::size_t>(sv.end() - begin)});
        return vec;
    }

    // bad news is that this is pulling fucking everything?
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

        munmap((void*)mapped_, size_);
    }

    const char* eof_{};
    private:
        int fd_{};
        const char* mapped_{};
        const char* curr_{};
        std::string path_{};
        std::size_t size_{};
};

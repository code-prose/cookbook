#pragma once
#include <string>
#include <exception>
#include <iostream>
#include <string_view>

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
        eof_ = mapped_ + size_ - 1;
        curr_ = mapped_;

        const char* eol = static_cast<const char*>(std::memchr(curr_, '\n', 1000));
        curr_ = eol + 1;
    }

    bool getline(std::string_view& sv) {
        auto bytes_left = eof_ - curr_;
        const char* eol = static_cast<const char*>(std::memchr(curr_, '\n', bytes_left));
        std::size_t sz = eol - curr_;
        // need to fix condition
        if (curr_ + 1 > (char*)eof_) return false;
        sv = std::string_view{curr_, sz};
        curr_ = eol + 1;
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

#pragma once
#include <string>
#include <exception>

#include <sys/mman.h>
#include <fcntl.h>
#include <sys/stat.h>

struct MappedFile {
    MappedFile(const std::string& path) : path_{path} {}
    void open() {
        fd_ = ::open(path_.c_str(), O_RDONLY);
        if (fd_ == -1) std::terminate();

        struct stat file_stats{};
        fstat(fd_, &file_stats);
        size_ = file_stats.st_size;
        // check result of fstat?

        mapped_ = mmap(0, file_stats.st_size, PROT_READ, MAP_PRIVATE, fd_, 0);
        if (mapped_ == MAP_FAILED) std::terminate();
        
        // need to strip csv headers... maybe this lives not in the struct
    }

    void close() {
        if (fd_ == -1) std::terminate();
        if (mapped_ == MAP_FAILED) std::terminate();

        munmap(mapped_, size_);
    }

    private:
        int fd_{};
        void* mapped_{};
        std::string path_{};
        std::size_t size_{};
};

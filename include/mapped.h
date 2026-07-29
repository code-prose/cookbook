#pragma once
#include <string>
#include <exception>

#include <sys/mman.h>
#include <fcntl.h>
#include <sys/stat.h>

struct MappedFile {
    MappedFile(const std::string& path) {
        auto fd = open(path.c_str(), O_RDONLY);
        if (fd == -1) std::terminate();
        struct stat file_stats{};
        fstat(fd, &file_stats);

        auto res = mmap(0, file_stats.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
        // for (auto itr = res; res != )

        if (res == MAP_FAILED) std::terminate();
    }

    private:
        void* mapped_{};
};

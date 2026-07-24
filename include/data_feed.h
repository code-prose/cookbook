#pragma once
#include <fstream>
#include <exception>
#include <iostream>

#include <sys/mman.h>
#include <fcntl.h>
#include <sys/stat.h>

#include "types.h"


class DataFeed {
public:

    DataFeed(const std::string& path) {
        // might need to specify some offset depending on how big the mapped file is

        // test_data.csv is 169359815 bytes,  ~161mb... I don't think I can map something this large
        auto fd = open(path.c_str(), O_RDONLY);
        if (fd == -1) std::terminate();
        struct stat file_stats{};
        fstat(fd, &file_stats);

        auto res = mmap(0, file_stats.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
        for (auto itr = res; res != )

        if (res == MAP_FAILED) std::terminate();
        // _fs = std::ifstream(path);
        // if (!_fs) throw std::runtime_error("Failed to open file: " + path);

        // discard headers
        std::getline(_fs, _line);
    }

    // how do I implement and iterator for mmap?
    friend struct Iterator;
    struct Iterator {
        DataFeed* _feed;
        Event _current;
        bool _done;

        const Event& operator*() const; 
        Iterator& operator++();
        bool operator!=(const Iterator& other) const;
    };


    Iterator begin();
    Iterator end();

private:
    std::ifstream _fs;
    std::string _line;
    Event ParseEvent();
};

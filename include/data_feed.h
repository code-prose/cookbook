#pragma once

#include <fstream>
#include "types.h"

class DataFeed {
public:

    DataFeed(const std::string& path) {
        _fs = std::ifstream(path);
        if (!_fs) throw std::runtime_error("Failed to open file: " + path);

        // discard headers
        std::string line;
        std::getline(_fs, line);
    }

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
    Event ParseEvent();
};

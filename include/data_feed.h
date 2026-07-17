#pragma once

#include <fstream>
#include "types.h"

class DataFeed {
public:

    DataFeed(const std::string& path) {
        _fs = std::ifstream(path);
        if (!_fs) throw std::runtime_error("Failed to open file: " + path);

        // discard headers
        std::getline(_fs, _line);
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
    std::string _line;
    Event ParseEvent();
};

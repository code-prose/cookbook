#pragma once

#include <fstream>
#include "types.h"

class DataFeed {
public:

    DataFeed(const std::string& path) {
        _fs = std::ifstream(path);
        std::string line;

        // discard headers
        std::getline(_fs, line);
    }

    struct Iterator {
        DataFeed* _feed;
        Event _current;
        bool _done;

        const Event& operator*() const; 
        Iterator& operator++();
        bool operator!=(const Iterator& other) const;
    };

    friend struct Iterator;


    Iterator begin();
    Iterator end();

private:
    std::ifstream _fs;
};

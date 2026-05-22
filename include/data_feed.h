#pragma once

#include <fstream>
#include "types.h"

class DataFeed {
public:

    DataFeed(const std::string& path) {
        _fs = std::ifstream(path);
    }

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
};

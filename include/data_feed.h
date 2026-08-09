#pragma once

#include <sys/mman.h>
#include <fcntl.h>
#include <sys/stat.h>

#include "types.h"
#include "mapped.h"


class DataFeed {
public:

    DataFeed(MappedFile& mfile) : mfile_{mfile} {
        mfile_.discard_headers();
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
    MappedFile mfile_;
    std::string line_;
    Event ParseEvent();
};

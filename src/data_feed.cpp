#pragma once

#include "types.h"
#include "data_feed.h"
#include <ranges>

const Event& DataFeed::Iterator::operator*() const {
    return _current;
};

DataFeed::Iterator& DataFeed::Iterator::operator++() {
    std::string line;
    std::getline(_feed->_fs, line);

    auto parts = line | std::views::split(',');
    // timestamp, id, type, trade_price, trade_quantity, side, ask_price, ask_quantity, bid_price, bid_quantity
    

};

DataFeed::Iterator DataFeed::begin() {

};

DataFeed::Iterator DataFeed::end() {

};

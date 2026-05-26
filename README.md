# cookbook

A C++23 order book and backtesting engine built for learning low-latency systems and quantitative finance infrastructure.

## Components

### Order Book (`include/order.h`, `src/order.cpp`)
A limit order book supporting limit, market, fill-or-kill, stop-loss, and buy-stop order types. Maintains separate bid and ask price levels, matches orders on insertion, and supports order cancellation and modification.

### Event Loop (`include/event_loop.h`, `src/event_loop.cpp`)
A discrete event simulation loop backed by a min-heap priority queue. Events are processed in timestamp order regardless of arrival order. Supports registering a single handler callback invoked for each event.

### Data Feed (`include/data_feed.h`, `src/data_feed.cpp`)
A CSV-backed market data feed that implements a C++ iterator interface for range-based for loop support. Parses trade and quote events from a wide-format CSV schema.

### Types (`include/types.h`)
Shared type definitions: `Price` (float), `Quantity` (uint32), `OrderID` (uint64), `Side`, `OrderType`, `TradeEvent`, `QuoteEvent`, `Event`.

## CSV Schema

```
timestamp, instrument, type, trade_price, trade_quantity, side, ask_price, ask_quantity, bid_price, bid_quantity
```

- `type` is either `trade` or `quote`
- Trade rows populate `trade_price`, `trade_quantity`, `side`; quote columns are empty
- Quote rows populate `ask_price`, `ask_quantity`, `bid_price`, `bid_quantity`; trade columns are empty
- Timestamps are nanoseconds since Unix epoch

## Generating Test Data

Requires Python with `polars` and `numpy`:

```bash
uv run python generate.py
```

Generates `test_data.csv` with a variable number rows of synthetic BTC-USD trade and quote events using a random walk price model.

## Building

Requires CMake 3.20+ and a C++23 compiler (Clang 17+ or GCC 14+).

```bash
cmake -B build -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
cmake --build build
ln -s build/compile_commands.json compile_commands.json  # for clangd
```

## Running

```bash
./build/cookbook
```

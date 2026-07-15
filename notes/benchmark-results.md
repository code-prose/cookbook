# data_feed benchmark results

Log of measured numbers for `DataFeed::ParseEvent` optimization work.
One row per change; change ONE thing, re-run, append.

## Setup
- Machine: macOS 15.7.3, Apple Silicon (arm64), `-O2 -g` Release build.
- Workload: `test_data.csv` rows repeated ~200x = **2,000,000 events**.
- Metric: end-to-end wall time (parse + event loop + strategy), µs/event.
- Commands:
  ```
  cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS="-O2 -g"
  cmake --build build -j
  cd <scratch-with-big-test_data.csv>
  /usr/bin/time -p <build>/cookbook >/dev/null        # wall time
  <build>/cookbook >/dev/null & sample $! 2 -mayDie    # profile
  ```

## Results

| date       | change                          | runs (s) real      | µs/event | notes |
|------------|---------------------------------|--------------------|----------|-------|
| 2026-07-14 | baseline (unmodified)           | 2.80 / 2.46 / 2.49 | ~1.25    | see profile below |
| 2026-07-14 | reuse `_line` member buffer     | 2.60 / 2.31 / 2.33 | ~1.17    | ~6% vs baseline (warm runs); removes getline alloc/event. `stringstream` still copies `_line`. user=2.22s |

## Baseline profile (2026-07-14, `sample`, 2s)
Leaf hotspots: `_platform_memmove` (string copies), `_free` + `_nanov2_free` (dealloc),
`_platform_memset`, `fastParse64` (libc number parse).

Hot lines in `ParseEvent`:
- L27 `getline` — 507
- L35 field `getline` — 410
- L36 `parts.push_back` — 182
- L30 `stringstream` ctor — 59
- L80/73/74 `stof`/`stoi`

## C1
- L27 `getline` - 414
- L35 field `getline` - 409
- L36 `parts.push_back` - 219
- L30 `stringsteam` ctor - 59
- L40 `Time` ctor - 57
- L80 `QuoteEvent` ctor - 35

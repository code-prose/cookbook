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


## Perf Results
```

 Performance counter stats for './build/microbench' (10 runs):

            139.38 msec task-clock:u                                                            ( +-  1.97% )
       636,589,344      cycles:u                                                                ( +-  0.43% )  (83.05%)
     2,214,346,469      instructions:u                                                          ( +-  0.08% )  (82.32%)
         5,388,005      cache-references:u                                                      ( +-  9.17% )  (83.38%)
            40,553      cache-misses:u                                                          ( +-  4.57% )  (83.86%)
       526,918,709      branches:u                                                              ( +-  0.06% )  (83.81%)
         1,571,404      branch-misses:u                                                         ( +-  0.22% )  (83.57%)

       0.140149011 +- 0.002758615 seconds time elapsed  ( +-  1.97% )
```

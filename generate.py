import polars as pl
import numpy as np
import datetime as dt

def main():
    # 0: timestamp, 1: id, 2: type, 3: trade_price, 4: trade_quantity, 5: side, 6: ask_price, 7: ask_quantity, 8: bid_price, 9: bid_quantity
    ROWS = 100
    base_time = np.datetime64("1970-01-01T00:00:00.500000000")
    timestamps = np.array([(base_time := base_time + dt.timedelta.microseconds(i)) for i in range(ROWS)])
    df = pl.DataFrame({"timestamp": timestamps}).with_columns(
        id=pl.lit(["TEST-ID"]),
        id=pl.lit(["buy", "sell"])

    )


if __name__ == "__main__":
    main()

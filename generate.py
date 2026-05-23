import polars as pl
import numpy as np


def main():
    # 0: timestamp, 1: id (instruments), 2: type, 3: trade_price, 4: trade_quantity, 5: side, 6: ask_price, 7: ask_quantity, 8: bid_price, 9: bid_quantity
    ROWS = 100
    start = np.datetime64("2024-01-01", "ns")
    step = np.timedelta64(1_000_000, "ns")
    timestamps = [int((start + i * step).astype("int64")) for i in range(ROWS)]
    instruments = ["TEST-ID" for _ in range(ROWS)]
    row_type = np.random.choice(["quote", "trade"], p=[0.7, 0.3], size=ROWS)
    sides = np.random.choice(["buy", "sell"], size=ROWS)

    price = [50000.0]
    for _ in range(1, ROWS):
        noise = np.random.normal(0, 1)
        price.append(price[-1] + noise)

    quantity = [150]
    for _ in range(1, ROWS):
        noise = np.random.randint(1, 100)
        quantity.append(quantity[-1] + noise)

    df = pl.DataFrame({
        "timestamp": timestamps,
        "id": instruments,
        "type": list(row_type),
        "price": price,
        "quantity": quantity,
        "sides": list(sides),
    }).with_columns(

        trade_price=pl
        .when(pl.col("type") == "trade")
        .then(pl.col("price"))
        .otherwise(None),

        trade_quantity=pl
        .when(pl.col("type") == "trade")
        .then(pl.col("quantity"))
        .otherwise(None),

        side=pl.when(pl.col("type") == "trade").then(pl.col("sides")).otherwise(None),

        ask_price=pl
        .when(pl.col("type") == "quote")
        .then(pl.col("price") + 0.5)
        .otherwise(None),

        ask_quantity=pl
        .when(pl.col("type") == "quote")
        .then(pl.col("quantity"))
        .otherwise(None),

        bid_price=pl
        .when(pl.col("type") == "quote")
        .then(pl.col("price") - 0.5)
        .otherwise(None),

        bid_quantity=pl
        .when(pl.col("type") == "quote")
        .then(pl.col("quantity"))
        .otherwise(None),
    )

    df = df.drop(["sides", "quantity", "price"])

    df.write_csv("./test_data.csv")


if __name__ == "__main__":
    main()

import polars as pl
import numpy as np
import argparse


def generate(rows: int = 1000):
    start = np.datetime64("2024-01-01", "ns")
    step = np.timedelta64(1_000_000, "ns")
    timestamps = [int((start + i * step).astype("int64")) for i in range(rows)]
    instruments = ["TEST-ID" for _ in range(rows)]
    row_type = np.random.choice(["quote", "trade"], p=[0.7, 0.3], size=rows)
    sides = np.random.choice(["buy", "sell"], size=rows)

    price = [50000.0]
    for _ in range(1, rows):
        noise = np.random.normal(0, 1)
        price.append(price[-1] + noise)

    quantity = [150]
    for _ in range(1, rows):
        noise = np.random.randint(1, 100)
        quantity.append(quantity[-1] + noise)

    # 0: timestamp, 1: id (instruments), 2: type, 3: trade_price, 4: trade_quantity, 5: side, 6: ask_price, 7: ask_quantity, 8: bid_price, 9: bid_quantity
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
    parser = argparse.ArgumentParser(
        prog='Market Data Generator',
        description='Generates CSV containing quote and trade events for pasing with backtesting engine',
        epilog=''
    )
    parser.add_argument('-r', '--rows')

    args = parser.parse_args()

    try:
        generate(rows=int(args.rows))
    except ValueError:
        print(f"Argument for --rows: {args.rows} is not an integer")

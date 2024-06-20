"""
Merge two results CSVs.
"""

import sys
import util

if __name__ == "__main__":
    if len(sys.argv) != 3:
        print("Usage: python merge.py <first.csv> <second.csv>")
        sys.exit(1)

    first = sys.argv[1]
    first = util.dictFromCSV(first)
    second = sys.argv[2]
    second = util.dictFromCSV(second)

    assert len(first) == len(second)

    firstKeys = list(first[0].keys())
    secondKeys = list(second[0].keys())
    # Both should start with the same primary key.
    assert firstKeys[0] == secondKeys[0]
    secondKeys = secondKeys[1:]

    intersection = set(firstKeys).intersection(set(secondKeys))
    if len(intersection) != 0:
        print(f"Duplicate columns found: {intersection}")
        print(
            "Please rename the columns in one of the CSVs, or check that the CSVs are provided."
        )
        sys.exit(1)

    resultKeys = firstKeys + secondKeys
    print(",".join(resultKeys))

    for firstRow, secondRow in zip(first, second):
        print(",".join([firstRow[key] for key in firstKeys]), end=",")
        print(",".join([secondRow[key] for key in secondKeys]))

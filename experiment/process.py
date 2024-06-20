from kresult import *
from plot import *
from util import *


def extractColumn(
    programs,
    ylabel,
    output,
    column=None,
    dictFun=None,  # E.g. lambda d: float(d["TSolver(s)"]) - float(d["TCex(s)"])
    # newNames=["S2G0", "S2G1"],
    newNames=["Mainline KLEE", "PoolUnsimp"],
    prefixes=[
        "s2gsimp",
        # "s2g-no-print",
        "poolunsimp",
        # "csa-hard-arrays",
        # "csa-hard-arrays-restart",
        # "soft-arrays-restart",
        # "csa-pooling",
        # "k-early",
        # "pooling-det-term",
        # "s2g-no-print-term",
    ],
    # suffixes=["base"],
    suffixes=["", ""],
    colours=DEFAULT_COLOURS,
):
    rcp = ResultCSVPrinter(["Program"] + newNames, output)
    for program in programs:
        if program == ALL_COREUTIL_PROGRAMS[2]:
            continue

        row = [program]
        for prefix, suffix in zip(prefixes, suffixes):
            if column:
                row.append(
                    extractResults(
                        f"{prefix}{program}{suffix}.klee-out.stats.csv"
                    )._dict[column]
                )
            else:
                row.append(
                    dictFun(
                        extractResults(
                            f"{prefix}{program}{suffix}.klee-out.stats.csv"
                        )._dict
                    )
                )
        rcp.writeRow(row)

    path = generateTex(rcp.csvFile, False, ylabel=ylabel, colours=colours)
    os.system("mkdir charts")
    os.system(f"pdflatex --output-directory=charts {path}")
    # Remove the aux files and the log files.
    os.system(f"rm -rf charts/*.aux")
    os.system(f"rm -rf charts/*.log")


def getInstrs(prefix="s2g-no-print", suffix="base", programs=ALL_COREUTIL_PROGRAMS):
    instrs = {}
    for program in programs:
        instrs[program] = extractResults(
            f"{prefix}{program}{suffix}.klee-out.stats.csv"
        ).instructions

        # other = extractResults(f"pooling{program}.klee-out.stats.csv").instructions

        # assert instrs[program] == other

    print(instrs)


def findPoolingSpeedupOverSolver2Global(programs=ALL_COREUTIL_PROGRAMS):
    speedups = []
    for program in programs:
        solver2Global = extractResults(f"s2g{program}a.klee-out.stats.csv").time
        pooling = extractResults(f"pooling-det-term{program}.klee-out.stats.csv").time
        speedups.append(solver2Global / pooling)

    # print median, mean:
    speedups = sorted(speedups)
    print(f"Median: {speedups[len(speedups) // 2]}")
    print(f"Mean: {sum(speedups) / len(speedups)}")


if __name__ == "__main__":
    # findPoolingSpeedupOverSolver2Global(
    #     programs=ALL_COREUTIL_PROGRAMS
    # )  # 1.44 median speed up!
    # exit()

    extractColumn(
        programs=ALL_COREUTIL_PROGRAMS,
        column="QCexCacheHits",
        output="QCexCacheHits",
        ylabel="QCexCacheHits",
    )

    extractColumn(
        programs=ALL_COREUTIL_PROGRAMS,
        column="QCacheHits",
        output="QCacheHits",
        ylabel="QCacheHits",
    )

    # exit()

    extractColumn(
        programs=ALL_COREUTIL_PROGRAMS,
        column="SolverQueries",
        output="SolverQueries",
        ylabel="SolverQueries",
    )

    extractColumn(
        programs=ALL_COREUTIL_PROGRAMS,
        column="SolverQueryConstructs",
        output="SolverQueryConstructs",
        ylabel="SolverQueryConstructs",
    )

    extractColumn(
        programs=ALL_COREUTIL_PROGRAMS,
        column="MaxActiveStates",
        output="MaxActiveStates",
        ylabel="MaxActiveStates",
    )

    extractColumn(
        programs=ALL_COREUTIL_PROGRAMS,
        # dictFun=lambda d: float(d["TCex(s)"]) - float(d["TQuery(s)"]),
        column="MaxMem(MiB)",
        output="Memory",
        ylabel="Max Memory (MiB)",
    )

    extractColumn(
        programs=ALL_COREUTIL_PROGRAMS,
        # dictFun=lambda d: float(d["TCex(s)"]) - float(d["TQuery(s)"]),
        column="Instrs",
        output="Instructions",
        ylabel="Instructions",
    )

    extractColumn(
        programs=ALL_COREUTIL_PROGRAMS,
        # dictFun=lambda d: float(d["TCex(s)"]) - float(d["TQuery(s)"]),
        column="TCex(s)",
        output="TCex",
        ylabel="Cex Time (s)",
    )

    extractColumn(
        programs=ALL_COREUTIL_PROGRAMS,
        column="Queries",
        output="Queries",
        ylabel="Queries",
    )

    extractColumn(
        programs=ALL_COREUTIL_PROGRAMS,
        column="TSolver(s)",
        output="TSolver",
        ylabel="Solver Time (s)",
    )

    extractColumn(
        programs=ALL_COREUTIL_PROGRAMS,
        column="TQuery(s)",
        output="TQuery",
        ylabel="Core Solver Time (s)",
    )

    extractColumn(
        programs=ALL_COREUTIL_PROGRAMS,
        column="Time(s)",
        output="Times",
        ylabel="Time (s)",
    )

    extractColumn(
        programs=ALL_COREUTIL_PROGRAMS,
        column="TSolver(%)",
        output="TSolverPercent",
        ylabel="Time spent in solver chain (\\%)",
    )

    extractColumn(
        programs=ALL_COREUTIL_PROGRAMS,
        dictFun=lambda d: float(d["TSolver(s)"]) - float(d["TCex(s)"]),
        output="SolverMinusCex",
        ylabel="Solver Time - Cex Time (s)",
    )

    extractColumn(
        programs=ALL_COREUTIL_PROGRAMS,
        dictFun=lambda d: float(d["TSolver(s)"]) - float(d["TQuery(s)"]),
        output="SolverMinusCore",
        ylabel="Solver Chain Time - Core Solver Time (s)",
    )
    # Let's measure the independence difference between the two.

    getInstrs()

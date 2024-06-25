from kresult import *
from plot import *
from util import *
from main import *

GOOD_PROGRAMS = [
    "base64",
    "dircolors",
    "du",
    "fold",
    "ln",
    "mkfifo",
    "nice",
    "od",
    "unexpand",
]


def extractColumn(
    programs,
    ylabel,
    output,
    column=None,
    dictFun=None,  # E.g. lambda d: float(d["TSolver(s)"]) - float(d["TCex(s)"])
    # newNames=["S2G0", "S2G1"],
    # newNames=["master-det"],
    # prefixes=["master-det"],
    # suffixes=["base"],
    newNames=["Mainline", "Solver2-Global", "Independent Pooling"],
    prefixes=["master-det-read-2", "global-s2-replay", "pooling-final"],
    suffixes=["", "", ""],
    # newNames=["S2-Global", "Pooling", "CSA", "CSA-Re", "CSA-Sore"],
    # prefixes=[
    #     "s2g",
    #     # "s2g-no-print",
    #     "pool",
    #     "csa-hard-arrays",
    #     "csa-hard-arrays-restart",
    #     "soft-arrays-restart",
    # ],
    # # suffixes=["base"],
    # suffixes=["a", "", "test", "test", "test"],
    colours=DEFAULT_COLOURS,
):
    # programs = ALL_COREUTIL_PROGRAMS + OTHERPROG
    rcp = ResultCSVPrinter(["Program"] + newNames, output)
    for program in programs:
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

    a = OTHERPROG[: len(programs)]

    for program in programs + a:
        instrs[program] = extractResults(
            f"{prefix}{program}{suffix}.klee-out.stats.csv"
        ).instructions

        # print(
        #     program,
        #     extractResults(f"{prefix}{program}{suffix}.klee-out.stats.csv").time,
        # )

        t = extractResults(f"{prefix}{program}{suffix}.klee-out.stats.csv").time
        if extractResults(f"{prefix}{program}{suffix}.klee-out.stats.csv").time < 4000:
            print("Program", program, "has time", t)

        CPP_INT_MAX = 2**31 - 1
        # assert instrs[program] < CPP_INT_MAX
        if instrs[program] >= CPP_INT_MAX:
            print(
                f"Program {program} has instructions >= {CPP_INT_MAX} with {instrs[program]} instructions."
            )

        # other = extractResults(f"pooling{program}.klee-out.stats.csv").instructions

        # assert instrs[program] == other

    print(instrs)
    print(len(instrs))


def findPoolingSpeedupOverSolver2Global(programs=GOOD_PROGRAMS):
    speedups = []
    for program in programs:
        solver2Global = extractResults(
            f"global-s2-replay{program}.klee-out.stats.csv"
        ).tSolverSeconds
        pooling = extractResults(
            f"pooling-final{program}.klee-out.stats.csv"
        ).tSolverSeconds
        speedups.append(solver2Global / pooling)

    print(speedups)
    # print median, mean:
    speedups = sorted(speedups)
    print(f"Median: {speedups[len(speedups) // 2]}")
    print(f"Mean: {sum(speedups) / len(speedups)}")


def findPoolingSpeedupOverMaster(programs=GOOD_PROGRAMS):
    speedups = []
    for program in programs:
        solver2Global = extractResults(
            f"master-det-read-2{program}.klee-out.stats.csv"
        ).tSolverSeconds
        pooling = extractResults(
            f"pooling-final{program}.klee-out.stats.csv"
        ).tSolverSeconds
        speedups.append(solver2Global / pooling)

    print(speedups)
    # print median, mean:
    speedups = sorted(speedups)
    print(f"Median: {speedups[len(speedups) // 2]}")
    print(f"Mean: {sum(speedups) / len(speedups)}")


def checkDeterminism(
    prefix1="master-det-read-2",
    prefix2="global-s2-replay",
    suffix1="",
    suffix2="",
    programs=ALL_COREUTIL_PROGRAMS,
):
    mp = {}
    for program in programs:
        r1 = extractResults(f"{prefix1}{program}{suffix1}.klee-out.stats.csv")
        r2 = extractResults(f"{prefix2}{program}{suffix2}.klee-out.stats.csv")

        # if program == "csplit":
        #     print(r1.queries, r2.queries)

        bad = False
        if r1.instructions != r2.instructions:
            bad = True
            print(f"Program {program} has different instructions.")

        if r1.queries != r2.queries:
            bad = True
            print(f"Program {program} has different queries.")

        if not bad:
            mp[program] = r1.instructions

        # if r1.maxActiveStates != r2.maxActiveStates:
        #     print(f"Program {program} has different max active states.")

    print(len(mp))
    print(mp)


def printKeyStatisticsAcrossProgramsToCSV(
    prefix,
    suffix="",
    programs=ALL_COREUTIL_PROGRAMS,
    columns=[
        "Time(s)",
        # "TCex(s)",
        "TSolver(s)",
        "TQuery(s)",
        "Queries",
        "SolverQueries",
        "ExternalCalls",
        "States",
        "MaxActiveStates",
        "TermEarly",
        "MaxMem(MiB)",
    ],
    columnNames=[
        "Time",
        # "Cex Time",
        "Solver Chain Time",
        "Core Solver Time",
        "Solver Chain Queries",
        "Core Solver Queries",
        "External Calls",
        "States",
        "Max Active States",
        "States Terminated Early",
        "Max Memory",
    ],
    output=None,
):
    if output is None:
        output = f"{prefix}{suffix}key-stats.csv"

    rcp = ResultCSVPrinter(["Program"] + columnNames, output)
    rows = []
    for program in programs:
        row = [program]
        for column in columns:
            row.append(
                extractResults(f"{prefix}{program}{suffix}.klee-out.stats.csv")._dict[
                    column
                ]
            )
        rows.append(row)
        rcp.writeRow(row)

    s = makeTable(rows)

    output = f"{prefix}{suffix}key-stats.tex"
    with open(output, "w") as f:
        f.write(s)

    os.system(f"pdflatex -interaction=nonstopmode {output}")


if __name__ == "__main__":
    # getInstrs(
    #     prefix="master-det",
    #     suffix="base",
    #     programs=ALL_COREUTIL_PROGRAMS + OTHERPROG,
    # )
    # P = FULL_COREUTIL_PROGRAMS + OTHERPROG[: len(FULL_COREUTIL_PROGRAMS)]

    findPoolingSpeedupOverMaster(programs=GOOD_PROGRAMS)
    findPoolingSpeedupOverSolver2Global(programs=GOOD_PROGRAMS)

    P = GOOD_PROGRAMS

    printKeyStatisticsAcrossProgramsToCSV(
        prefix="master-det-read-2",
        suffix="",
        programs=P,
    )

    printKeyStatisticsAcrossProgramsToCSV(
        prefix="global-s2-replay",
        suffix="",
        programs=P,
    )

    printKeyStatisticsAcrossProgramsToCSV(
        prefix="pooling-final",
        suffix="",
        programs=P,
    )

    # exit()

    # P = [prog for prog in P if prog != "factor"]
    # checkDeterminism(
    #     prefix1="master-det-read-2",
    #     # prefix2="global-s2-replay",
    #     prefix2="pooling-final",
    #     # suffix="base,
    #     suffix1="",
    #     suffix2="",
    #     programs=GOOD_PROGRAMS,
    # )
    # exit()

    # findPoolingSpeedupOverSolver2Global(
    #     programs=ALL_COREUTIL_PROGRAMS
    # )  # 1.75 speed up!
    # getInstrs()

    P = GOOD_PROGRAMS

    findPoolingSpeedupOverMaster(programs=P)
    findPoolingSpeedupOverSolver2Global(programs=P)
    # exit()

    extractColumn(
        programs=P,
        column="SolverQueries",
        output="SolverQueries",
        ylabel="SolverQueries",
    )

    extractColumn(
        programs=P,
        column="SolverQueryConstructs",
        output="SolverQueryConstructs",
        ylabel="SolverQueryConstructs",
    )

    extractColumn(
        programs=P,
        column="MaxActiveStates",
        output="MaxActiveStates",
        ylabel="MaxActiveStates",
    )

    extractColumn(
        programs=P,
        # dictFun=lambda d: float(d["TCex(s)"]) - float(d["TQuery(s)"]),
        column="MaxMem(MiB)",
        output="Memory",
        ylabel="Max Memory (MiB)",
    )

    extractColumn(
        programs=P,
        # dictFun=lambda d: float(d["TCex(s)"]) - float(d["TQuery(s)"]),
        column="Instrs",
        output="Instructions",
        ylabel="Instructions",
    )

    extractColumn(
        programs=P,
        # dictFun=lambda d: float(d["TCex(s)"]) - float(d["TQuery(s)"]),
        column="TCex(s)",
        output="TCex",
        ylabel="Cex Time (s)",
    )

    extractColumn(
        programs=P,
        column="Queries",
        output="Queries",
        ylabel="Queries",
    )

    extractColumn(
        programs=P,
        column="TSolver(s)",
        output="TSolver",
        ylabel="Solver Time (s)",
    )

    extractColumn(
        programs=P,
        column="TQuery(s)",
        output="TQuery",
        ylabel="Core Solver Time (s)",
    )

    # P = (FULL_COREUTIL_PROGRAMS + OTHERPROG[: len(FULL_COREUTIL_PROGRAMS)],)

    extractColumn(
        programs=P,
        column="Time(s)",
        output="Times",
        ylabel="Time (s)",
    )

    extractColumn(
        programs=P,
        column="TSolver(%)",
        output="TSolverPercent",
        ylabel="Time spent in solver chain (\\%)",
    )

    extractColumn(
        programs=P,
        dictFun=lambda d: float(d["TSolver(s)"]) - float(d["TCex(s)"]),
        output="SolverMinusCex",
        ylabel="Solver Time - Cex Time (s)",
    )

    extractColumn(
        programs=P,
        dictFun=lambda d: float(d["TSolver(s)"]) - float(d["TQuery(s)"]),
        output="SolverMinusCore",
        ylabel="Solver Chain Time - Core Solver Time (s)",
    )

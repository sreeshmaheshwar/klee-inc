import os

from enum import Enum
from kresult import extractResults
from util import DUMP_DIR, KLEE_BIN_PATH


class Solver(Enum):
    Z3 = "z3"
    STP = "stp"


class IncrementalityLevel(Enum):
    NonIncremental = ""
    Incremental = "--use-incremental"
    Solver2Local = "--incremental-baseline"
    Solver2Global = "--global-incremental-baseline"


class SearchStrategy(Enum):
    DefaultHeuristic = "--search=random-path --search=nurs:covnew"
    Inputting = "--search=inputting"
    # DefaultHeuristic = "--search=random-path"
    DFS = "--search=dfs"
    BFS = "--search=bfs"


EXCEPTIONS = {
    "dd": "--sym-args 0 3 10 --sym-files 1 8 --sym-stdin 8 --sym-stdout",
    "dircolors": "--sym-args 0 3 10 --sym-files 2 12 --sym-stdin 12 --sym-stdout",
    "echo": "--sym-args 0 4 300 --sym-files 2 30 --sym-stdin 30 --sym-stdout",
    "expr": "--sym-args 0 1 10 --sym-args 0 3 2 --sym-stdout",
    "mknod": "--sym-args 0 1 10 --sym-args 0 3 2 --sym-files 1 8 --sym-stdin 8 --sym-stdout",
    "od": "--sym-args 0 3 10 --sym-files 2 12 --sym-stdin 12 --sym-stdout",
    "pathchk": "--sym-args 0 1 2 --sym-args 0 1 300 --sym-files 1 8 --sym-stdin 8 --sym-stdout",
    "printf": "--sym-args 0 3 10 --sym-files 2 12 --sym-stdin 12 --sym-stdout",
}


def symArgsForProgram(name: str) -> str:
    return EXCEPTIONS.get(
        name,
        "--sym-args 0 1 10 --sym-args 0 2 2 --sym-files 1 8 --sym-stdin 8 --sym-stdout",
    )


DEFAULT_MEMORY = 2000  # TODO.


def run(
    name,
    solver: Solver,
    inc: IncrementalityLevel,
    searchStrategy: SearchStrategy,
    cex,
    independent,
    branch,
    memory,
    dirName=None,
    timeToRun=None,
    instructions=None,
    removeOutput=True,
    debugDumpZ3=False,
    simplify=True,
    solverTimeout=None,  # NOTE.
    dumpStates=False,  # NOTE.
    batchingInstrs=None,
    externalCalls="concrete",  # NOTE.
    additionalOptions="",
    logger=None,
    logFile=None,
):
    assert timeToRun is not None or instructions is not None

    if timeToRun is None:
        timeToRun = 0

    watchdog = timeToRun != 0  # Only use watchdog if we are enforcing max-time.

    if instructions is None:
        instructions = 0

    sym_args = symArgsForProgram(name)

    if dirName is None:
        dirName = f"{name}{solver.name}{inc.name}{additionalOptions}.klee-out"
        dirName = DUMP_DIR + "/" + dirName

    # We want to git ignore all dumps so let's put them in a directory.
    # Make the dump directory if it doesn't exist.
    os.system(f"mkdir {DUMP_DIR}")
    # Remove existing output directory, if it exists.
    os.system(f"rm -rf {dirName}")

    debugDumpZ3Option = f"--debug-z3-dump-queries={dirName}.dump" if debugDumpZ3 else ""
    solverTimeoutOption = f"--max-solver-time={solverTimeout}" if solverTimeout else ""
    batchOption = (
        f"--use-batching-search --batch-instructions={batchingInstrs}"
        if batchingInstrs
        else ""
    )
    searchOption = searchStrategy.value + " " + batchOption
    incOption = inc.value
    logOption = "--use-query-log=all:smt2" if logFile is not None else ""

    runCommand = f"{KLEE_BIN_PATH}/klee --env-file=../src/test.env --run-in-dir=/tmp/sandbox --output-dir={dirName} --solver-backend={solver.value} {solverTimeoutOption} --simplify-sym-indices {logOption} --write-cvcs --write-cov --output-module --max-memory={memory} --disable-inlining --optimize --use-forked-solver --use-cex-cache={cex} --use-independent-solver={independent} --use-branch-cache={branch} --rewrite-equalities={simplify}  --libc=uclibc --posix-runtime --external-calls={externalCalls} --only-output-states-covering-new --max-sym-array-size=4096 --max-time={timeToRun}s --watchdog={watchdog} --max-instructions={instructions} --max-memory-inhibit=false --max-static-fork-pct=1 --max-static-solve-pct=1 --max-static-cpfork-pct=1 --switch-type=internal --dump-states-on-halt={dumpStates} {searchOption} {incOption} {debugDumpZ3Option} {additionalOptions} ../src/{name}.bc {sym_args}"

    print(runCommand)

    return

    if logger is not None:
        import datetime

        now = datetime.datetime.now()
        logger.logAndPrint(f"At {now}, running command...\n{runCommand}")

    os.system(runCommand)

    # Save stats CSV.
    klee_stats_path = f"{dirName}.stats.csv"
    command = f"{KLEE_BIN_PATH}/klee-stats --table-format=csv --print-all {dirName} > {klee_stats_path}"
    os.system(command)

    # Extract results from the CSV.
    results = extractResults(csvPath=klee_stats_path)

    if logFile is not None:
        logPath = f"{dirName}/all-queries.smt2"
        os.system(f"mv {logPath} {logFile}")

    if removeOutput:
        os.system(f"rm -rf {dirName}")

    return results


def optRun(
    name,
    inc,
    searchStrategy,
    solver=Solver.Z3,
    cex=1,
    independent=1,
    branch=1,
    dirName=None,
    timeToRun=None,
    instructions=None,
    removeOutput=True,
    debugDumpZ3=False,
    simplify=True,
    solverTimeout=None,
    batchingInstrs=None,
    externalCalls="concrete",
    dumpStates=False,
    additionalOptions="",
    logger=None,
    logFile=None,
    memory=DEFAULT_MEMORY,
):
    return run(
        name=name,
        solver=solver,
        inc=inc,
        searchStrategy=searchStrategy,
        cex=cex,
        independent=independent,
        branch=branch,
        dirName=dirName,
        timeToRun=timeToRun,
        instructions=instructions,
        removeOutput=removeOutput,
        debugDumpZ3=debugDumpZ3,
        simplify=simplify,
        solverTimeout=solverTimeout,
        batchingInstrs=batchingInstrs,
        externalCalls=externalCalls,
        dumpStates=dumpStates,
        additionalOptions=additionalOptions,
        logger=logger,
        logFile=logFile,
        memory=memory,
    )


def unOptRun(
    name,
    inc,
    searchStrategy,
    simplify,
    solver=Solver.Z3,
    cex=0,
    independent=0,
    branch=0,
    dirName=None,
    timeToRun=None,
    instructions=None,
    removeOutput=True,
    debugDumpZ3=False,
    solverTimeout=None,
    batchingInstrs=None,
    externalCalls="concrete",
    dumpStates=False,
    additionalOptions="",
    logger=None,
    logFile=None,
    memory=DEFAULT_MEMORY,
):
    return run(
        name=name,
        solver=solver,
        inc=inc,
        searchStrategy=searchStrategy,
        cex=cex,
        independent=independent,
        branch=branch,
        dirName=dirName,
        timeToRun=timeToRun,
        instructions=instructions,
        removeOutput=removeOutput,
        debugDumpZ3=debugDumpZ3,
        simplify=simplify,
        solverTimeout=solverTimeout,
        externalCalls=externalCalls,
        batchingInstrs=batchingInstrs,
        dumpStates=dumpStates,
        additionalOptions=additionalOptions,
        logger=logger,
        logFile=logFile,
        memory=memory,
    )


if __name__ == "__main__":
    optRun(
        inc=IncrementalityLevel.NonIncremental,
        name="base64",
        searchStrategy=SearchStrategy.DefaultHeuristic,
        batchingInstrs=10000,
        memory=1500,
        dirName=f"output.klee-out",
        timeToRun=int(70 * 60),  # NOTE.
        # logger=logger,
        # removeOutput=False,
        # logFile=f"{branch}{p}new.log",
        additionalOptions=f"--state-output=states --tr-output=tr",
    ).instructions
    # from util import buildBranch

    # buildBranch("master")

    # results = unOptRun(
    #     name="ln",
    #     solver=Solver.Z3,
    #     inc=IncrementalityLevel.NonIncremental,
    #     searchStrategy=SearchStrategy.DFS,
    #     timeToRun=10,
    #     simplify=False,
    # )

    # print(results.time)

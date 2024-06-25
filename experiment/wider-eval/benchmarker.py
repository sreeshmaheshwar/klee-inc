"""
An example benchmark implementation.

Note that the number of implementations that
we compare may be changed - it does not have
to be just Baseline and Testee.
"""

# Branch: TODO: Enter branch name.

from util import *
from runklee import *

# It is useful to set a timeout for all KLEE runs: in the
# unexpected case of a run severely underperforming (and
# being worse than the benchmark), we don't want our entire
# experiment to hang.
MAX_TIMEOUT_MINS = 25  # TODO.

# If set instructions are required, we may use them here, and otherwise
# we get the instructions to run for from a run of baseline KLEE.
OLD_INSTRS = [
    2874421,
    53744551,
    589149889,
    31457154,
    150188056,
    312337991,
    131884472,
    690001309,
    69857371,
    390247,
    141228204,
    537540062,
]

# In the case that we get instructions from a baseline KLEE run, we
# may want to scale the received instructions to deal with over-running
# post-halt.
INSTRS_SCALE_FACTOR = 0.85  # TODO.


# Sample benchmarking, for instructions, that compares non-incremental
# and incremental KLEE runs for a list of programs, with constraint
# set simplification enabled for both.
def benchmark_instrs(timeInMins, programs, experimentName="results"):
    logger = ProgressLogger()
    printer = ResultCSVPrinter(
        # NOTE: Can be extended to more columns / testees.
        columnNames=["Program", "Non-Incremental Time", "Incremental Time"],
        experimentName=experimentName,
    )

    # Maintain query mismatches for determinism checks, instruction mismatches
    # for error / early termination / bug checking, and instruction list for
    # rerunning and getting set instruction values for future experiments.
    queryMismatches, instrMismatches, instrList = [], [], []

    for i, program in enumerate(programs):
        # Retrieve instructions from the supplied list, if desired:
        instrs = getInstrsForProgram(
            name=program,
            coreInstructions=OLD_INSTRS,
            corePrograms=programs,  # NOTE: This may be changed to, e.g. ALL_COREUTIL_PROGRAMS, if we're running a subset of a previous run
        )

        instrList.append(instrs)

        # Rerun baseline on found instructions.
        nonIncResults = optRun(
            inc=IncrementalityLevel.NonIncremental,
            name=program,
            timeToRun=MAX_TIMEOUT_MINS * 60,
            instructions=instrs,
            searchStrategy=SearchStrategy.DFS,
            simplify=True,
            logger=logger,
        )

        # Run testee on found instructions.
        incResults = optRun(
            inc=IncrementalityLevel.Incremental,
            name=program,
            timeToRun=MAX_TIMEOUT_MINS * 60,
            instructions=instrs,
            searchStrategy=SearchStrategy.DFS,
            simplify=True,
            logger=logger,
        )

        if nonIncResults.queries != incResults.queries:
            queryMismatches.append((program, nonIncResults.queries, incResults.queries))

        if nonIncResults.instructions != incResults.instructions:
            instrMismatches.append(
                (program, nonIncResults.instructions, incResults.instructions)
            )

        logger.logAndPrint(
            f"""Results for {program} ({i + 1} / {len(programs)}):
    - (First Time: {nonIncResults.time}, Second Time: {incResults.time})
    - Current query mismatches: {queryMismatches}
    - Current instruction mismatches: {instrMismatches}"""
        )

        printer.writeRow([program, nonIncResults.time, incResults.time])

    logger.logAndPrint(f"Printing results...\n{printer.read()}")
    logger.logAndPrint(f"Printing instruction list:\n{instrList}")
    logger.logAndPrint(f"Printing query mismatches:\n{queryMismatches}")
    logger.logAndPrint(f"Printing instruction mismatches:\n{instrMismatches}")
    logger.persistToFile(experimentName)


# Sample benchmarking, where instructions are found from a timed run, that
# compares non-incremental and incremental KLEE runs for a list of programs,
# with constraint set simplification enabled for both.
def benchmark_time(timeInMins, programs, experimentName="results"):
    logger = ProgressLogger()
    printer = ResultCSVPrinter(
        # NOTE: Can be extended to more columns / testees.
        columnNames=["Program", "Non-Incremental Time", "Incremental Time"],
        experimentName=experimentName,
    )

    # Maintain query mismatches for determinism checks, instruction mismatches
    # for error / early termination / bug checking, and instruction list for
    # rerunning and getting set instruction values for future experiments.
    queryMismatches, instrMismatches, instrList = [], [], []

    for i, program in enumerate(programs):
        # Baseline KLEE run to get instructions:
        instrs = round(
            optRun(
                inc=IncrementalityLevel.NonIncremental,
                name=program,
                timeToRun=int(timeInMins * 60),
                searchStrategy=SearchStrategy.DFS,
                simplify=True,
                logger=logger,
            ).instructions
            * INSTRS_SCALE_FACTOR
        )

        instrList.append(instrs)

        # Rerun baseline on found instructions.
        nonIncResults = optRun(
            inc=IncrementalityLevel.NonIncremental,
            name=program,
            timeToRun=MAX_TIMEOUT_MINS * 60,
            instructions=instrs,
            searchStrategy=SearchStrategy.DFS,
            simplify=True,
            logger=logger,
        )

        # Run testee on found instructions.
        incResults = optRun(
            inc=IncrementalityLevel.Incremental,
            name=program,
            timeToRun=MAX_TIMEOUT_MINS * 60,
            instructions=instrs,
            searchStrategy=SearchStrategy.DFS,
            simplify=True,
            logger=logger,
        )

        if nonIncResults.queries != incResults.queries:
            queryMismatches.append((program, nonIncResults.queries, incResults.queries))

        if nonIncResults.instructions != incResults.instructions:
            instrMismatches.append(
                (program, nonIncResults.instructions, incResults.instructions)
            )

        logger.logAndPrint(
            f"""Results for {program} ({i + 1} / {len(programs)}):
    - (First Time: {nonIncResults.time}, Second Time: {incResults.time})
    - Current query mismatches: {queryMismatches}
    - Current instruction mismatches: {instrMismatches}"""
        )

        printer.writeRow([program, nonIncResults.time, incResults.time])

    logger.logAndPrint(f"Printing results...\n{printer.read()}")
    logger.logAndPrint(f"Printing instruction list:\n{instrList}")
    logger.logAndPrint(f"Printing query mismatches:\n{queryMismatches}")
    logger.logAndPrint(f"Printing instruction mismatches:\n{instrMismatches}")
    logger.persistToFile(experimentName)


def benchmark(timeInMins, programs, experimentName="results"):
    # benchmark_time(timeInMins, programs, experimentName)
    # benchmark_instrs(timeInMins, programs, experimentName)
    pass

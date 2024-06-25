from util import *
from runklee import *

VM_ID = int(os.getenv("VM_ID"))

MAX_TIMEOUT_MINS = 20  # If it's longer than this, it's probably timed out.


# DFS Instructions.
# IS = {
#     "base64": 49368497,
#     "chmod": 487609795,
#     "comm": 29441932,
#     "csplit": 426165810,
#     "dircolors": 383172627,
#     "factor": 84331408,
#     "ln": 84473060 - 200,
#     "mkfifo": 569949874,
# }

IS = {
    "base64": 4465283,
    "chmod": 9790168,
    "comm": 7521661,
    "csplit": 10877792,
    "dircolors": 1847469,
    "factor": 765257,
    "ln": 396832,
    # "mkfifo": 6553854,
    "mkfifo": 4553854,
    # "mkfifo": 1486984,  # NOTE.
    # "mkfifo": 1546487,
    # "mkfifo": 15631782, # NOTE: For random path.
}


def getBaseRun(
    time,
    programs,
    branches=["s2g-no-print-det"],
):
    logger = ProgressLogger()
    for i, branch in enumerate(branches):
        buildBranch(branch)
        p = "mkfifo"
        print(
            optRun(
                inc=IncrementalityLevel.NonIncremental,
                name=p,
                # instructions=IS[p],
                searchStrategy=SearchStrategy.DefaultHeuristic,
                dirName=f"{branch}{p}base.klee-out",
                timeToRun=time * 60,
                logger=logger,
                # removeOutput=False,
                # logFile=f"{branch}{p}new.log",
            ).instructions
            - 200
        )
    pass


def runWithPrograms(
    time,
    programs,
    branches=["s2g-no-print"],
):
    assert 0 <= VM_ID < len(programs)
    assert len(programs) == len(IS)

    logger = ProgressLogger()

    times = []
    for i, branch in enumerate(branches):
        buildBranch(branch)
        p = "mkfifo"
        times.append(
            optRun(
                inc=IncrementalityLevel.NonIncremental,
                name=p,
                instructions=IS[p],
                searchStrategy=SearchStrategy.DefaultHeuristic,
                dirName=f"{branch}{p}new.klee-out",
                timeToRun=2 * 60 * 60,  # 2 hours
                logger=logger,
                removeOutput=False,
                logFile=f"{branch}{p}new.log",
            ).time
        )

    print(times)


def forOutput(
    time,
    programs,
    branches=["s2g-no-print"],
):
    assert 0 <= VM_ID < len(programs)
    assert len(programs) == len(IS)

    logger = ProgressLogger()

    # times = []
    for i, branch in enumerate(branches):
        buildBranch(branch)
        p = "mkfifo"
        q1 = optRun(
            inc=IncrementalityLevel.NonIncremental,
            name=p,
            instructions=IS[p],
            searchStrategy=SearchStrategy.DefaultHeuristic,
            dirName=f"{branch}{p}old.klee-out",
            timeToRun=2 * 60 * 60,  # 2 hours
            logger=logger,
            # removeOutput=False,
            # logFile=f"{branch}{p}new.log",
            additionalOptions=f"--state-output=/home/{USERNAME}/coreutils-6.10/obj-llvm/klee-test/states.txt",
        ).queries
        q2 = optRun(
            inc=IncrementalityLevel.NonIncremental,
            name=p,
            instructions=IS[p],
            searchStrategy=SearchStrategy.Inputting,
            dirName=f"{branch}{p}new.klee-out",
            timeToRun=2 * 60 * 60,  # 2 hours
            logger=logger,
            # removeOutput=False,
            logFile=f"{branch}{p}new.log",
            additionalOptions=f"--state-input=/home/{USERNAME}/coreutils-6.10/obj-llvm/klee-test/states.txt",
        ).queries

        print(q1, q2)

    # print(times)


if __name__ == "__main__":
    # runWithPrograms(time=10, programs=ALL_COREUTIL_PROGRAMS)
    forOutput(time=10, programs=ALL_COREUTIL_PROGRAMS)

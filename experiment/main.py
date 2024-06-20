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

# IS = {
#     "base64": 4465283,
#     "chmod": 9790168,
#     "comm": 7521661,
#     "csplit": 10877792,
#     "dircolors": 1847469,
#     "factor": 765257,
#     "ln": 396832,
#     # "mkfifo": 6553854,
#     "mkfifo": 4553854,
#     # "mkfifo": 1486984,  # NOTE.
#     # "mkfifo": 1546487,
#     # "mkfifo": 15631782, # NOTE: For random path.
# }

# IS = {
#     "base64": 20382436,
#     "chmod": 82714816,
#     "comm": 12846639,
#     "csplit": 74543082,
#     "dircolors": 1411454,
#     "factor": 822777,
#     "ln": 678212,
#     "mkfifo": 94984825,
# }

# IS = {
#     "base64": 8305490,
#     "chmod": 17414590,
#     "comm": 9091075,
#     "csplit": 19231645,
#     "dircolors": 2344517,
#     "factor": 1116602,
#     "ln": 376763,
#     "mkfifo": 20299794,
# }


def runBaseWithPrograms(
    time,
    programs,
    branches=["master-det2"],
):
    assert 0 <= VM_ID < len(programs)

    logger = ProgressLogger()

    for i, branch in enumerate(branches):
        buildBranch(branch)
        p = programs[VM_ID]
        ins = (
            optRun(
                inc=IncrementalityLevel.NonIncremental,
                name=p,
                searchStrategy=SearchStrategy.DefaultHeuristic,
                memory=1500,
                batchingInstrs=1000,
                dirName=f"{branch}{p}base.klee-out",
                timeToRun=int(time * 60),  # NOTE.
                logger=logger,
                # removeOutput=False,
                # logFile=f"{branch}{p}new.log",
                additionalOptions=f"--state-output=/home/{USERNAME}/bstates.txt --tr-output=/home/{USERNAME}/btr.txt",
            ).instructions
            - 200
        )

        q1 = optRun(
            inc=IncrementalityLevel.NonIncremental,
            name=p,
            # instructions=IS[p],
            searchStrategy=SearchStrategy.Inputting,
            memory=3000,
            dirName=f"s2gsimp{p}.klee-out",
            timeToRun=MAX_TIMEOUT_MINS * 60,
            instructions=ins,
            logger=logger,
            # removeOutput=False,
            # logFile=f"{branch}{p}new.log",
            additionalOptions=f"--state-input=/home/{USERNAME}/bstates.txt --tr-input=/home/{USERNAME}/btr.txt",
        ).queries

        buildBranch("pooling-det-term")
        q2 = optRun(
            inc=IncrementalityLevel.NonIncremental,
            name=p,
            simplify=False,
            # instructions=IS[p],
            searchStrategy=SearchStrategy.Inputting,
            memory=3000,
            dirName=f"poolunsimp{p}.klee-out",
            timeToRun=MAX_TIMEOUT_MINS * 60,
            instructions=ins,
            logger=logger,
            # removeOutput=False,
            # logFile=f"{branch}{p}new.log",
            additionalOptions=f"--state-input=/home/{USERNAME}/bstates.txt --tr-input=/home/{USERNAME}/btr.txt --pool-size=10",
        ).queries

        print()
        print(q1, q2)


def runWithProgram(
    programs,
    # branches=["s2g-no-print", "s2g-no-print"],
    branches,
):
    assert 0 <= VM_ID < len(programs)

    logger = ProgressLogger()

    for i, branch in enumerate(branches):
        buildBranch(branch)
        p = programs[VM_ID]
        optRun(
            inc=IncrementalityLevel.NonIncremental,
            name=p,
            # instructions=IS[p],
            searchStrategy=SearchStrategy.Inputting,
            memory=3000,
            dirName=f"{branch}{p}.klee-out",
            timeToRun=MAX_TIMEOUT_MINS * 60,
            instructions=IS[p],
            logger=logger,
            # removeOutput=False,
            # logFile=f"{branch}{p}new.log",
            # additionalOptions=f"--state-input=/home/{USERNAME}/nb-states.txt --tr-input=/home/{USERNAME}/test-output.txt --csa-timeout=200 --pool-size=10",
            additionalOptions=f"--state-input=/home/{USERNAME}/nb-states.txt --tr-input=/home/{USERNAME}/test-output.txt --pool-size=8",  # - pool-det-term
            # additionalOptions=f"--state-input=/home/{USERNAME}/nb-states.txt --tr-input=/home/{USERNAME}/test-output.txt",
            simplify=False,  # TODO.
        )


if __name__ == "__main__":
    # runWithPrograms(time=10, programs=ALL_COREUTIL_PROGRAMS)
    # runWithProgram(programs=FULL_COREUTIL_PROGRAMS, branches=["csa-hard-arrays"])
    # runWithProgram(
    #     programs=FULL_COREUTIL_PROGRAMS, branches=["csa-hard-arrays-restart"]
    # )
    # runWithProgram(programs=FULL_COREUTIL_PROGRAMS, branches=["soft-arrays-restart"])
    # runWithProgram(programs=FULL_COREUTIL_PROGRAMS, branches=["soft-arrays-timeout"])
    # runWithProgram(programs=FULL_COREUTIL_PROGRAMS, branches=["csa-pooling"])
    # runWithProgram(programs=FULL_COREUTIL_PROGRAMS, branches=["k-early"])
    runBaseWithPrograms(time=11, programs=FULL_COREUTIL_PROGRAMS)
    # runWithProgram(programs=FULL_COREUTIL_PROGRAMS, branches=["s2g-no-print-term"])

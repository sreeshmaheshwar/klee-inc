from util import *
from runklee import *

VM_ID = int(os.getenv("VM_ID"))

MAX_TIMEOUT_MINS = 100  # If it's longer than this, it's probably timed out.


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

IS = {
    "base64": 143698238,
    "dircolors": 10858522,
    "ln": 6355212,
    "mkfifo": 2122518592,
    "seq": 48810836,
    "od": 316558102,
    "fold": 36998646,
    "unexpand": 34456067,
    "kill": 1089170759,
    "nice": 1763019281,
    "du": 10912153,
}

OTHERPROG = [
    "seq",
    "od",
    "fold",
    # "[",
    "unexpand",
    "dd",
    "kill",
    "nice",
    "du",
    "tsort",
    "expr",
    "ptx",
    "expand",
    "printf",
    "tr",
    "rm",
    "fmt",
    "rmdir",
    "link",
    "uname",
    "sleep",
    "stty",
    "readlink",
    "id",
]


def runBaseWithPrograms(
    time,
    programs,
    branches=["master-det-read-2"],
):
    assert 0 <= VM_ID < len(programs)
    # assert len(programs) == len(OTHERPROG) // 3

    a = OTHERPROG[: len(programs)]
    # b = OTHERPROG[len(programs) : 2 * len(programs)]
    # c = OTHERPROG[2 * len(programs) :]

    assert len(a) == len(programs)

    # assert len(b) == len(programs)
    # assert len(c) == len(programs)

    logger = ProgressLogger()

    for i, branch in enumerate(branches):
        buildBranch(branch)
        for ps in [programs, a]:
            p = ps[VM_ID]
            ins = (
                optRun(
                    inc=IncrementalityLevel.NonIncremental,
                    name=p,
                    searchStrategy=SearchStrategy.DefaultHeuristic,
                    batchingInstrs=10000,
                    memory=1500,
                    dirName=f"{branch}{p}base.klee-out",
                    timeToRun=int(time * 60),  # NOTE.
                    logger=logger,
                    # removeOutput=False,
                    # logFile=f"{branch}{p}new.log",
                    additionalOptions=f"--state-output=/home/{USERNAME}/{p}new-states --tr-output=/home/{USERNAME}/{p}new-tr",
                ).instructions
                - 200
            )

            r1 = optRun(
                inc=IncrementalityLevel.NonIncremental,
                name=p,
                searchStrategy=SearchStrategy.Inputting,
                memory=3000,
                dirName=f"{branch}{p}.klee-out",
                timeToRun=MAX_TIMEOUT_MINS * 60,
                instructions=ins,
                logger=logger,
                additionalOptions=f"--state-input=/home/{USERNAME}/{p}new-states.gz --tr-input=/home/{USERNAME}/{p}new-tr.gz",
            )

            # assert r1.instructions == ins

            buildBranch("global-s2-replay")  # TODO.

            r2 = optRun(
                inc=IncrementalityLevel.NonIncremental,
                name=p,
                searchStrategy=SearchStrategy.Inputting,
                memory=3000,
                dirName=f"global-s2-replay{p}.klee-out",
                timeToRun=MAX_TIMEOUT_MINS * 60,
                instructions=ins,
                logger=logger,
                additionalOptions=f"--state-input=/home/{USERNAME}/{p}new-states.gz --tr-input=/home/{USERNAME}/{p}new-tr.gz",
            )

            # assert r2.instructions == ins

            print(r1.queries, r2.queries)

            # return  # TODO.


def runWithProgram(
    programs,
    branch="pooling-final",
):
    assert 0 <= VM_ID < len(programs)
    # assert len(programs) == len(OTHERPROG) // 3

    a = OTHERPROG[: len(programs)]
    # b = OTHERPROG[len(programs) : 2 * len(programs)]
    # c = OTHERPROG[2 * len(programs) :]

    assert len(a) == len(programs)

    # assert len(b) == len(programs)
    # assert len(c) == len(programs)

    logger = ProgressLogger()

    buildBranch(branch)
    for ps in [programs, a]:
        p = ps[VM_ID]

        if p not in IS:
            continue

        ins = IS[p]

        r1 = optRun(
            inc=IncrementalityLevel.NonIncremental,
            name=p,
            searchStrategy=SearchStrategy.Inputting,
            memory=3000,
            dirName=f"{branch}{p}.klee-out",
            timeToRun=MAX_TIMEOUT_MINS * 60,
            instructions=ins,
            logger=logger,
            additionalOptions=f"--state-input=/home/{USERNAME}/{p}new-states.gz --tr-input=/home/{USERNAME}/{p}new-tr.gz --inc-timeout=1000",
        )


if __name__ == "__main__":
    # runWithPrograms(time=10, programs=ALL_COREUTIL_PROGRAMS)
    # runWithProgram(programs=FULL_COREUTIL_PROGRAMS, branches=["csa-hard-arrays"])
    # runWithProgram(
    #     programs=FULL_COREUTIL_PROGRAMS, branches=["csa-hard-arrays-restart"]
    # )
    # runWithProgram(programs=FULL_COREUTIL_PROGRAMS, branches=["soft-arrays-restart"])
    # runBaseWithPrograms(70, programs=FULL_COREUTIL_PROGRAMS)
    runWithProgram(programs=FULL_COREUTIL_PROGRAMS)

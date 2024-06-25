"""
Some testing utilities.
"""

import csv
import os

DUMP_DIR = "dump"
RESULTS_DIR = "results"
PROGRESS_DIR = "progress"

USERNAME = os.getenv("KLEE_USERNAME")
KLEE_PATH = f"/home/{USERNAME}/klee"  # TODO: Retrieve this from environment instead?
KLEE_BIN_PATH = f"{KLEE_PATH}/build/bin"

ALL_COREUTIL_PROGRAMS = [
    "base64",
    "chmod",
    "comm",
    "csplit",
    "dircolors",
    # "echo",
    # "env",
    "factor",
    "ln",
    "mkfifo",
]

SAMPLE_COREUTIL_PROGRAMS = ALL_COREUTIL_PROGRAMS[:2]

FULL_COREUTIL_PROGRAMS = [
    "base64",
    "chmod",
    "comm",
    "csplit",
    "dircolors",
    # "echo",
    # "env",
    "factor",
    "ln",
    "mkfifo",
]


def buildBranch(branchName):
    curDir = os.getcwd()
    os.chdir(f"{KLEE_PATH}/build")
    os.system("git fetch --all")
    os.system(f"git checkout {branchName}")
    os.system("git pull")
    os.system("make")
    os.chdir(curDir)


def getInstrsForProgram(name, coreInstructions, corePrograms):
    """
    If the supplied instruction list, ordered, corresponds to the
    supplied programs list, returns the instruction corresponding
    to the given program.
    """
    assert len(coreInstructions) == len(corePrograms)
    return coreInstructions[corePrograms.index(name)]


def dictFromCSV(csvPath: str):
    with open(csvPath, "r") as file:
        csv_reader = csv.DictReader(file)
        data = [row for row in csv_reader]
        return data


class ProgressLogger(object):
    def __init__(self, progressFile="progress.txt"):
        os.system(f"mkdir {DUMP_DIR}")
        self.progressFile = f"{DUMP_DIR}/{progressFile}"
        with open(self.progressFile, "w+") as f:
            f.write("")

    def log(self, progress):
        with open(self.progressFile, "a") as f:
            f.write(progress + "\n\n\n")

    def logAndPrint(self, progress):
        print("\n\n" + progress + "\n\n")
        self.log(progress)

    def persistToFile(self, file, appendTxt=True, prependProgress=True):
        if appendTxt:
            file = f"{file}.txt"

        if prependProgress:
            os.system(f"mkdir {PROGRESS_DIR}")
            file = f"{PROGRESS_DIR}/{file}"

        with open(file, "w") as dest:
            with open(self.progressFile, "r") as src:
                dest.write(src.read())


class ResultCSVPrinter(object):
    def __init__(self, columnNames, experimentName, appendCsv=True):
        if appendCsv:
            experimentName = f"{experimentName}.csv"

        os.system(f"mkdir {RESULTS_DIR}")
        self.columns = columnNames
        self.csvFile = f"{RESULTS_DIR}/{experimentName}"

        # Create the CSV file with the column names.
        with open(self.csvFile, "w+") as f:
            f.write(",".join(columnNames) + "\n")

    def writeRow(self, row):
        assert len(row) == len(self.columns)
        with open(self.csvFile, "a") as f:
            f.write(",".join(list(map(str, row))) + "\n")

    def read(self):
        with open(self.csvFile, "r") as f:
            return f.read()

import json

from util import dictFromCSV


# This code is largely generated from a script.
class KResults:

    def __init__(self, _dict: dict) -> None:
        self._dict = _dict

    @property
    def time(self) -> float:
        return float(self._dict["Time(s)"])

    @property
    def instructions(self) -> int:
        return int(self._dict["Instrs"])

    @property
    def iCovPercent(self) -> float:
        return float(self._dict["ICov(%)"])

    @property
    def bCovPercent(self) -> float:
        return float(self._dict["BCov(%)"])

    @property
    def iCount(self) -> int:
        return int(self._dict["ICount"])

    @property
    def tSolverPercent(self) -> float:
        return float(self._dict["TSolver(%)"])

    @property
    def iCovered(self) -> int:
        return int(self._dict["ICovered"])

    @property
    def iUncovered(self) -> int:
        return int(self._dict["IUncovered"])

    @property
    def branches(self) -> int:
        return int(self._dict["Branches"])

    @property
    def fullBranches(self) -> int:
        return int(self._dict["FullBranches"])

    @property
    def partialBranches(self) -> int:
        return int(self._dict["PartialBranches"])

    @property
    def externalCalls(self) -> int:
        return int(self._dict["ExternalCalls"])

    @property
    def tUserSeconds(self) -> float:
        return float(self._dict["TUser(s)"])

    @property
    def tResolveSeconds(self) -> float:
        return float(self._dict["TResolve(s)"])

    @property
    def tResolvePercent(self) -> float:
        return float(self._dict["TResolve(%)"])

    @property
    def tCexSeconds(self) -> float:
        return float(self._dict["TCex(s)"])

    @property
    def tCexPercent(self) -> float:
        return float(self._dict["TCex(%)"])

    @property
    def tQuerySeconds(self) -> float:
        return float(self._dict["TQuery(s)"])

    @property
    def tSolverSeconds(self) -> float:
        return float(self._dict["TSolver(s)"])

    @property
    def states(self) -> int:
        return int(self._dict["States"])

    @property
    def activeStates(self) -> int:
        return int(self._dict["ActiveStates"])

    @property
    def maxActiveStates(self) -> int:
        return int(self._dict["MaxActiveStates"])

    @property
    def avgActiveStates(self) -> float:
        return float(self._dict["AvgActiveStates"])

    @property
    def inhibitedForks(self) -> int:
        return int(self._dict["InhibitedForks"])

    @property
    def queries(self) -> int:
        return int(self._dict["Queries"])

    @property
    def solverQueries(self) -> int:
        return int(self._dict["SolverQueries"])

    @property
    def solverQueryConstructs(self) -> int:
        return int(self._dict["SolverQueryConstructs"])

    @property
    def qCacheMisses(self) -> int:
        return int(self._dict["QCacheMisses"])

    @property
    def qCacheHits(self) -> int:
        return int(self._dict["QCacheHits"])

    @property
    def qCexCacheMisses(self) -> int:
        return int(self._dict["QCexCacheMisses"])

    @property
    def qCexCacheHits(self) -> int:
        return int(self._dict["QCexCacheHits"])

    @property
    def allocations(self) -> int:
        return int(self._dict["Allocations"])

    @property
    def memMiB(self) -> float:
        return float(self._dict["Mem(MiB)"])

    @property
    def maxMemMiB(self) -> float:
        return float(self._dict["MaxMem(MiB)"])

    @property
    def avgMemMiB(self) -> float:
        return float(self._dict["AvgMem(MiB)"])

    @property
    def brConditional(self) -> int:
        return int(self._dict["BrConditional"])

    @property
    def brIndirect(self) -> int:
        return int(self._dict["BrIndirect"])

    @property
    def brSwitch(self) -> int:
        return int(self._dict["BrSwitch"])

    @property
    def brCall(self) -> int:
        return int(self._dict["BrCall"])

    @property
    def brMemOp(self) -> int:
        return int(self._dict["BrMemOp"])

    @property
    def brResolvePointer(self) -> int:
        return int(self._dict["BrResolvePointer"])

    @property
    def brAlloc(self) -> int:
        return int(self._dict["BrAlloc"])

    @property
    def brRealloc(self) -> int:
        return int(self._dict["BrRealloc"])

    @property
    def brFree(self) -> int:
        return int(self._dict["BrFree"])

    @property
    def brGetVal(self) -> int:
        return int(self._dict["BrGetVal"])

    @property
    def termExit(self) -> int:
        return int(self._dict["TermExit"])

    @property
    def termEarly(self) -> int:
        return int(self._dict["TermEarly"])

    @property
    def termSolverErr(self) -> int:
        return int(self._dict["TermSolverErr"])

    @property
    def termProgrErr(self) -> int:
        return int(self._dict["TermProgrErr"])

    @property
    def termUserErr(self) -> int:
        return int(self._dict["TermUserErr"])

    @property
    def termExecErr(self) -> int:
        return int(self._dict["TermExecErr"])

    @property
    def termEarlyAlgo(self) -> int:
        return int(self._dict["TermEarlyAlgo"])

    @property
    def termEarlyUser(self) -> int:
        return int(self._dict["TermEarlyUser"])

    @property
    def tArrayHashSeconds(self) -> float:
        return float(self._dict["TArrayHash(s)"])

    @property
    def tForkSeconds(self) -> float:
        return float(self._dict["TFork(s)"])

    @property
    def tForkPercent(self) -> float:
        return float(self._dict["TFork(%)"])

    @property
    def tUserPercent(self) -> float:
        return float(self._dict["TUser(%)"])

    def to_json(self) -> str:
        properties = vars(self)
        json_dict = {prop: getattr(self, prop) for prop in properties}
        json_dict = json_dict["_dict"]
        return json.dumps(json_dict, indent=2)


def extractResults(csvPath: str):
    data = dictFromCSV(csvPath=csvPath)
    assert len(data) == 1
    data = data[0]
    return KResults(data)


if __name__ == "__main__":
    import sys

    assert len(sys.argv) == 2
    path = sys.argv[1]

    res = extractResults(path)
    print(res.time)
    print(res.to_json())

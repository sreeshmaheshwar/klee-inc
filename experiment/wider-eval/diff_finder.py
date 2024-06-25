from dataclasses import dataclass


"""Example file:
; Query 0 -- Type: Truth, Instructions: 12099
(set-logic QF_AUFBV )
(declare-fun n_args () (Array (_ BitVec 32) (_ BitVec 8) ) )
(assert (bvult  (concat  (select  n_args (_ bv3 32) ) (concat  (select  n_args (_ bv2 32) ) (concat  (select  n_args (_ bv1 32) ) (select  n_args (_ bv0 32) ) ) ) ) (_ bv2 32) ) )
(check-sat)
(exit)
;   OK -- Elapsed: 1.011424e-02s
;   Is Valid: false

; Query 1 -- Type: Validity, Instructions: 12108
(set-logic QF_AUFBV )
(declare-fun n_args () (Array (_ BitVec 32) (_ BitVec 8) ) )
(assert (let ( (?B1 (concat  (select  n_args (_ bv3 32) ) (concat  (select  n_args (_ bv2 32) ) (concat  (select  n_args (_ bv1 32) ) (select  n_args (_ bv0 32) ) ) ) ) ) ) (and  (=  false (bvslt  (_ bv0 32) ?B1 ) ) (bvult  ?B1 (_ bv2 32) ) ) ) )
(check-sat)
(exit)
;   OK -- Elapsed: 7.638910e-04s
;   Validity: 0

; Query 2 -- Type: Truth, Instructions: 12455
(set-logic QF_AUFBV )
(declare-fun n_args () (Array (_ BitVec 32) (_ BitVec 8) ) )
(declare-fun n_args_1 () (Array (_ BitVec 32) (_ BitVec 8) ) )
(assert (let ( (?B1 (concat  (select  n_args (_ bv3 32) ) (concat  (select  n_args (_ bv2 32) ) (concat  (select  n_args (_ bv1 32) ) (select  n_args (_ bv0 32) ) ) ) ) ) ) (and  (and  (bvult  (concat  (select  n_args_1 (_ bv3 32) ) (concat  (select  n_args_1 (_ bv2 32) ) (concat  (select  n_args_1 (_ bv1 32) ) (select  n_args_1 (_ bv0 32) ) ) ) ) (_ bv3 32) ) (bvult  ?B1 (_ bv2 32) ) ) (=  false (bvslt  (_ bv0 32) ?B1 ) ) ) ) )
(check-sat)
(exit)
;   OK -- Elapsed: 9.548510e-04s
;   Is Valid: false

; Query 3 -- Type: Validity, Instructions: 12464
(set-logic QF_AUFBV )
(declare-fun n_args () (Array (_ BitVec 32) (_ BitVec 8) ) )
(declare-fun n_args_1 () (Array (_ BitVec 32) (_ BitVec 8) ) )
(assert (let ( (?B1 (concat  (select  n_args_1 (_ bv3 32) ) (concat  (select  n_args_1 (_ bv2 32) ) (concat  (select  n_args_1 (_ bv1 32) ) (select  n_args_1 (_ bv0 32) ) ) ) ) ) (?B2 (concat  (select  n_args (_ bv3 32) ) (concat  (select  n_args (_ bv2 32) ) (concat  (select  n_args (_ bv1 32) ) (select  n_args (_ bv0 32) ) ) ) ) ) ) (and  (and  (and  (=  false (bvslt  (_ bv0 32) ?B1 ) ) (bvult  ?B2 (_ bv2 32) ) ) (=  false (bvslt  (_ bv0 32) ?B2 ) ) ) (bvult  ?B1 (_ bv3 32) ) ) ) )
(check-sat)
(exit)
;   OK -- Elapsed: 6.939510e-04s
;   Validity: 0
"""


@dataclass
class QueryWithType:
    i: int  # Number
    t: str  # Type
    instructions: int  # Instructions
    result: str = ""  # Result if query is Truth or Validity type, "" otherwise.


def get_query(line):
    l = line.split()
    return QueryWithType(int(l[2]), l[5][: len(l[5]) - 1], int(l[7]))


def queriesFromFile(file):
    with open(file, "r") as f:
        comments = [line for line in f.readlines() if line.startswith(";")]
        ret = []
        for i, comment in enumerate(comments):
            if comment.startswith("; Query"):
                temp = get_query(comment)
                if temp.t == "Truth":
                    l = comments[i + 2].split()
                    assert l[2] == "Valid:" and l[3] in ["true", "false"]
                    ret.append(QueryWithType(temp.i, temp.t, temp.instructions, l[3]))
                elif temp.t == "Validity":
                    l = comments[i + 2].split()
                    assert l[1] == "Validity:" and l[2] in ["1", "-1", "0"]
                    ret.append(QueryWithType(temp.i, temp.t, temp.instructions, l[2]))
                else:
                    ret.append(temp)

        assert ret[-1].i == len(ret) - 1
        return ret


def find_diff(file1, file2):
    l1 = queriesFromFile(file1)
    l2 = queriesFromFile(file2)

    print(f"Read {len(l1)} queries from {file1}")
    print(f"Read {len(l2)} queries from {file2}")

    for q1, q2 in zip(l1, l2):
        if q1.t != q2.t or q1.instructions != q2.instructions or q1.result != q2.result:
            print(f"First difference at query {q1.i}")
            return

    print("No differences found!")


# Finds the diff only comparing types and instructions, not results.
def find_diff_types_instructions(file1, file2):
    with open(file1, "r") as f1:
        ls = f1.readlines()
        l1 = [get_query(line) for line in ls if line.startswith("; Query")]
        print(f"Read {len(l1)} queries from {file1}")

        with open(file2, "r") as f2:
            ls = f2.readlines()
            l2 = [get_query(line) for line in ls if line.startswith("; Query")]
            print(f"Read {len(l2)} queries from {file2}")

            # Find first type difference between the two
            firstType, lastType = None, None
            firstInstr, lastInstr = None, None
            for q1, q2 in zip(l1, l2):
                if q1.t != q2.t:
                    firstType = q1.i if firstType is None else firstType
                if q1.instructions != q2.instructions:
                    firstInstr = q1.i if firstInstr is None else firstInstr

            for q1, q2 in zip(reversed(l1), reversed(l2)):
                if q1.t != q2.t:
                    lastType = (q1.i, q2.i) if lastType is None else lastType
                if q1.instructions != q2.instructions:
                    lastInstr = (q1.i, q2.i) if lastInstr is None else lastInstr

            assert firstType is not None and lastType is not None
            print(
                f"firstType type difference at query {firstType}, lastType at queries {lastType} respectively"
            )

            assert firstInstr is not None and lastInstr is not None
            print(
                f"firstInstr type difference at query {firstInstr}, lastInstr at queries {lastInstr} respectively"
            )

    pass


if __name__ == "__main__":
    import sys

    find_diff_types_instructions(sys.argv[1], sys.argv[2])

"""
Command:

python3 diff_finder.py noninc_comments.log inc_comments.log

Output:

Read 82584 queries from noninc_comments.log
Read 82592 queries from inc_comments.log
firstType type difference at query 7546, lastType at queries (45883, 45891) respectively
firstInstr type difference at query 7546, lastInstr at queries (82583, 82591) respectively
"""

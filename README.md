# KLEE Individual Project

## Source Code

We use branches extensively in this proejct, as we comparatively investigate different strategies. Also, existing KLEE is quite large so we strongly recommend looking at the corresponding PR or branch `diff` instead of each branch's code in isolation.

See below for a list of branches corresponding to strategies mentioned in the report.

### Heuristic-Tested Strategies

These strategies were all tested for default, heurstic search (and some with batching too) and so all support deterministic replaying of a search strategy and the determistic outputting of heuristic search approach. Please see the "Wider Evaluation" section code for the branches used for the wider evaluation (where we had to change our configuration slightly to read through buffers).

- Independent $K$-Pooling: https://github.com/sreeshmaheshwar/klee/pull/23
- Master / Mainline KLEE: https://github.com/sreeshmaheshwar/klee/pull/28
- Solver2 Global Baseline: https://github.com/sreeshmaheshwar/klee/pull/24
- K-Early: https://github.com/sreeshmaheshwar/klee/pull/27
- CSA-Pooling/optimised CSA: https://github.com/sreeshmaheshwar/klee/pull/26 

Note that optimisations (caches and constraint set simplification) can be enabled and disabled via **command line options** (not different branches) - but for evaluating please see the next section.

### Other Strategies

Here we find all other strategies (that were used in the report for DFS). Some may support deterministic state replaying but most do not as it is unneeded for DFS. Note that many strategies above orgiginated from DFS ones so this is only those used for DFS that were **not** used elsewhere.

- CSA (unoptmisied): https://github.com/sreeshmaheshwar/klee/pull/3
- Simplify-Early variant of Partition-Early: https://github.com/sreeshmaheshwar/klee/pull/21
- Partition-Early (simplification can be toggled via command line option `--rewrite-equalities`): https://github.com/sreeshmaheshwar/klee/pull/14
- LCP-PP: https://github.com/sreeshmaheshwar/klee/pull/19
- Stack-Basic: https://github.com/sreeshmaheshwar/klee/pull/10
- Basic-Stack Write-New: https://github.com/sreeshmaheshwar/klee/pull/10
- Basic-Stack Assert Variant (we mention in the report, on page 30, that we implement a strategy that errors based on constraint set prefixes to double check `basic-stack`): https://github.com/sreeshmaheshwar/klee/pull/7

## Evaluation/Experiment Code

We provide in this repository an `experiment` directory that contains an example of how we ran an example *preliminary experiment* (that is, using our eight benchmarks and ~10 minutes baselines). The exact experiment is not one eventually included but should showcase the generalisable methodology we used. We also importantly provide the code used to generate the **wider evaluation results** separately (see next section).

Note that we use VMs to run a single program over one VM - this can be modified to run all programs if a single machine is used. The experiment can be run with `python3 main.py`, and graphs and statistics can be automatically produced with `process.py` afterwards. Whichever combination of strategies can be tested against each other by changing the branch names so long as they are compatible (hence we provide just one such experiment). We also provide flexible infrastructure for enabling and disabling command line arguments.

**Before** testing, run the script `bash setup.sh` to configure your machhine deterministically (though do inspect this first). It also requires the instructions (mentioned below to ahve been carried out).

**Note that setting everything up is difficult**. We provide the *full instructions* used to configure a Ubuntu 20.04 machine (one of our eight Imperial VMs) entirely from scratch for the Coreutils experiments within `experiment/setup-instructions.md`. Some steps (e.g. setting up a key for my development) may not be relevent. If you are using 22.04, see [here](https://klee-se.org/docs/coreutils-experiments/) instead for just installing required dependencies.

We note that for our DFS experiments, we do not need to output and input state replays since it is deterministic. So this command line option should be changed (and `SearchStrategy.DFS` used instead).

## Wider Evaluation

See `experiment/wider-eval` for the code used to generate the wider evaluation results. We note that within `main.py` the function `runBaseWithPrograms` was used to run the baseline and then rerun the *Master* and *Solver2-Global* strategies. Then the function `runWtihProgram` was used to rerun *Independent K-Pooling*. We bring particular attention to **the branches used** - as mentioned in the report, we had to modify our deterministic replayer to input in a buffered way due to memory issues (luckily ZLIB allowed us to do this) and these branches reflect this change (though we believe there to be some bugs that cause corruption of the term replay `.gz` file that forced us to discard some programs).

The wider evaluation code works similarly to the preliminary code with `main.py` and `process.py` (but you'll need to first wun `runBaseWithPrograms` to generate the logs for our replayer). We bring attention to the different functions within `process.py` and the differing `main` function. In particular, the wider evaluation supports the key run statistics shown in our appendices **being automatically converted to the TEX tables provided** (along with the execution time chart as before). This file also finds and prints the speedup statistics obtained from the runs and `checkDeterminism` shows how we checked for those programs compatible with our deterministic replayer (that we call *good* programs).

### Results Differences

**Versions:** Versions (e.g. Z3 version) can hugely vary results produced. The `setup-instructions.md` that we provide include specific versions (e.g. Z3 version 4.13.0 - 64 bit) (and in the report).

**Machine:** Results will also vary based on your machine (hardware specifications + OS).

**Runs:** Certain runs are very non-deterministic. For example, different values returned by the underlying solver can actually cause noticeable time differences between identically-configured runs due to caching.

---

Original fork README is below:

---

KLEE Symbolic Virtual Machine
=============================

[![Build Status](https://github.com/klee/klee/workflows/CI/badge.svg)](https://github.com/klee/klee/actions?query=workflow%3ACI)
[![Build Status](https://api.cirrus-ci.com/github/klee/klee.svg)](https://cirrus-ci.com/github/klee/klee)
[![Coverage](https://codecov.io/gh/klee/klee/branch/master/graph/badge.svg)](https://codecov.io/gh/klee/klee)

`KLEE` is a symbolic virtual machine built on top of the LLVM compiler
infrastructure. Currently, there are two primary components:

  1. The core symbolic virtual machine engine; this is responsible for
     executing LLVM bitcode modules with support for symbolic
     values. This is comprised of the code in lib/.

  2. A POSIX/Linux emulation layer oriented towards supporting uClibc,
     with additional support for making parts of the operating system
     environment symbolic.

Additionally, there is a simple library for replaying computed inputs
on native code (for closed programs). There is also a more complicated
infrastructure for replaying the inputs generated for the POSIX/Linux
emulation layer, which handles running native programs in an
environment that matches a computed test input, including setting up
files, pipes, environment variables, and passing command line
arguments.

For further information, see the [webpage](http://klee.github.io/).

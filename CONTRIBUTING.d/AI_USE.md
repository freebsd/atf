# AI Use

## Introduction

AI use and submissions in FreeBSD projects is (transparently) a bit of a gray
area right now.  In general, the maintainer of this project permits AI use on a
case-by-case basis, depending on the size and type of contribution.

Please note that the FreeBSD's project's stance on AI/LLMs is evolving and the
terms in this section can change at a later date as the FreeBSD Project
formalizes its stance on AI/LLM use.

## Human are Responsible for all Contributions

If you...
* ... cannot logically explain your submission...
* ... cannot vouch that the submission does not violate other copyrights or
  licenses, e.g., the submission contains GPL-licensed code, uncited
  documentation from Wikipedia, etc...

... your contribution (Issue, Pull Request, etc) will be promptly closed.

[Human] Authors take responsibility for their contributions: not AI/LLMs.

## Commit Metadata

### Assisted-by

Contributions made with the assistance of AI/LLMs, must be called out in
commits using the `Assisted-by` label, referencing the LLM name and version
used in the contribution, e.g.,
```
Assisted-by: Claude:claude-opus-4-8
```
__This is enforced by commit-check__

### Authors

**The author of a specific piece of code cannot be AI.**

As implied in earlier sections, AI/LLMs cannot attest or take responsibility for
works submitted to the project.

__This is enforced by commit-check__

### Co-Authored-By
**`Co-Authored-By:` attributions in commits is strictly forbidden**.

As implied in the earlier sections, humans own any commits submitted to the ATF
project and are responsible for such, as described in the
[BSD 2-Clause License](https://opensource.org/license/bsd-2-clause).

__This is enforced by commit-check__

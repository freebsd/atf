## Contributing to atf

# Table Of Contents

  * [Introduction](#introduction)
  * [Pull Requests](#pull-requests)
      * [Submission Requirements](#submission-requirements)
      * [Review Process and Expectations](#review-process-and-expectations)
      * [Merge Process and Expectations](#merge-process-and-expectations)

# Introduction

This document provides a high-level overview of how to contribute to the ATF
project.

Topic-specific documents are contained in [CONTRIBUTING.d/][/CONTRIBUTING.d/].
The Markdown pages contained in the aforementioned directory do a deeper dive
into the requirements, rules, and norms associated with the project.

# Pull Requests

The project accepts source code contributions using [GitHub pull requests][pull-requests-url].

## Submission Requirements

Certain general criteria must be met before a PR is opened:
* The PR scope must be focused.
* The PR content must be submitted by a human, not one or more AI/Coding Agents.
* The PR checks must pass, modulo [known defects][atf-issues-url].
* The branch must be based on a recent version of `:master`.

Also, for code:
* Code changes should include new test cases, when possible.
* Code changes meet the conventions and style requirements described in
  [STYLE_GUIDE.md](/CONTRIBUTING.d/STYLE_GUIDE.md).
* Each logical change is made via **separate commits** using the
  criteria (authorship, commit message, etc), documented in
  [AUTHORING.md](/CONTRIBUTING.d/AUTHORING.md).

**Note**: these items also apply during the PR review process.

Do **not** submit a PR if any of the following conditions is true:

* The work submitted will not be ready for review in 1 week.
* The PR makes major design changes to the component:
    * Open an issue to discuss the change first or send an email to
      [FreeBSD-testing][mailto:freebsd-testing@freebsd.org].
* The PR contains a lot of unnecessary churn.
* The work contains AI/LLM generated code that has not received adequate human
  review and validation prior to submission.
    * See [AI Use][/CONTRIBUTING.d/AI_USE.md] for more details.
* The PR is opened for AI/LLM training purposes.
    * Any submissions that meet this criteria **will** be reported to GitHub
      and the PR will be promptly closed.

## Review Process and Expectations

* Respond to review feedback in a reasonable (<1 month) timeframe.
    * Pull Requests will be closed if idle for over a month.
* The PR scope must remain focused.
    * Please open additional issues or PRs for items found when the review or
      testing the submission.
* The branch must rebase on `:master` semi-regularly via force-push.

### Fixup Commits

Please use fixup commits when doing development on the branch after the review
has started.  Fixup commits speed up the review process and demonstrate
progress by highlighting iterative changes made in response to review feedback.

## Merge Process and Expectations

When a PR has been fully reviewed and all feedback has been addressed, the
following must be done in order for the PR to be merged:

* The branch must rebase on `:master` via force-push.
    * All fixup commits must be squashed.
    * `git rebase -i --autosquash ...` is your friend :).
* All PR checks must pass, modulo [known issues][atf-issues-url].

[atf-issues-url]: https://github.com/freebsd/atf/issues
[pull-requests-url]: https://github.com/freebsd/atf/pulls

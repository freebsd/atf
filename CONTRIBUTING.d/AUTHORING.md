# Authoring

## Commit Requirements

Each commit must:

* Follow the [commit log message guide][commit-log-message-guide-url].
* Include one or more `Signed-off-by:` trailers certifying
  [Developer Certificate of Origin][dco-url].
    * `Signed-off-by` must be done via humans, not LLMs.
* AI/LLM contributions must be disclosed.
    * See [AI Use](/CONTRIBUTING.d/AI_USE.md) for more details.
* Include `Closes: #<github-issue>` if fixing a GitHub Issue open in
  [freebsd/atf][atf-issues-url].
* Commits must use your desired name/moniker.
    * See [Author Name](#author-name) for more details.
* Commits must be made using valid, non-anonymous email addresses.
    * See [Email Addresses](#email-addresses) for more details.

### Author Name

All submissions must be made using sufficiently identifiable names. Names like
`Charlie Root`, or other misleading names like `Abraham Lincoln`, for instance,
will not be accepted.

The name used in your submissions does not need to match your legal name. It
needs to be a name or moniker which helps address/identify you, in combination
with your email address.

[FreeBSD Core](mailto:core@freebsd.org) may request authors of significant
changes using an obvious pseudonym to document their identity more concretely
should there be issues in the future. The author may request this documentation
be kept confidential unless needed for legal issues arising from their
contributions.

### Email Addresses

All commits must be authored and signed off using a non-anonymous GitHub email
address, e.g., not email addresses like `jane.doe@users.noreply.github.com`.

__This is enforced by commit-check__

#### Rationale

Authoring and signing off commits using a non-anonymous email address ensures
the authors have enough information to contact the authors if questions arise
about their contributions.

### Licensing

All code must have a license consistent with the project as a whole, unless the
code comes from a third-party source, e.g., [autoconf-archive][], in which case
the licensing and authorship information remain, per the terms of the original
license.

[autoconf-archive]: https://github.com/autoconf-archive
[commit-check-url]: https://commit-check.com/
[commit-log-message-guide-url]: https://docs.freebsd.org/en/articles/committers-guide/#commit-log-message
[dco-url]: https://en.wikipedia.org/wiki/Developer_Certificate_of_Origin

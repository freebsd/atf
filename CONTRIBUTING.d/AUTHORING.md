# Authoring

## Commit Requirements

Each commit must:

* Follow the [commit log message guide][commit-log-message-guide-url].
* Include one or more `Signed-off-by:` trailers certifying
  [Developer Certificate of Origin][dco-url].
    * `Signed-off-by` must be done via humans, not LLMs.
* LLM contributions must be disclosed.
    * See [LLM Use](#llm-use) for more details.
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

[commit-check-url]: https://commit-check.com/
[commit-log-message-guide-url]: https://docs.freebsd.org/en/articles/committers-guide/#commit-log-message

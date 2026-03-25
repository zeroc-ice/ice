# Copilot PR Review Instructions

## Binary Compatibility Check

**When reviewing PRs, check if changes break public API binary compatibility.**

Binary compatibility includes changes such as:

- Removing or renaming public classes, methods, or functions
- Changing method signatures (parameters, return types)
- Changing class hierarchy or virtual table layout
- Removing or reordering data members of public types
- Changing enum values or constants

### Rules by branch:

- **`main` branch**: Breaking binary compatibility is **allowed**.
- **Release branches** (e.g., `3.7`, `3.8`): Breaking binary compatibility is **not allowed**.

### Template if binary compatibility is broken on a release branch:

```
This PR targets a release branch and includes changes that break public API binary compatibility.
Binary compatibility must be preserved on release branches.

Breaking change(s):
- [Description of the breaking change]
```

## Changelog Validation

**When reviewing PRs, check if code changes have a CHANGELOG.md entry.**

### Requires changelog entry:

- Non-trivial bug fixes, features, API changes, security fixes, performance improvements
- Breaking changes, deprecations
- Multi-language changes

### Doesn't require entry:

- Comment/whitespace-only changes
- Internal refactoring with no behavioral changes
- Test-only, CI/CD, and build system changes
- Internal documentation

### Template if missing:

```
This PR has [description], but doesn't update CHANGELOG.md.
Please add an entry describing this change.

Generated suggestion:
- [Generated description based on the changes made]
```

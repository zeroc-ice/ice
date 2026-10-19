# Changelog Fragments

Unreleased changelog entries live in this directory as fragment files, one file per PR, instead of being added to
the shared changelog file. Different PRs add different files, so changelog entries never produce merge conflicts or
duplicate section headings. At release time, `scripts/collate_changelog.py` collates the fragments into the
changelog.

## Adding a fragment

A PR that warrants a changelog entry adds `changelog.d/<section>/<id>.md`, where:

- `<section>` is one of the directories listed below and selects the changelog section — the collation script
  generates the section headings from it.
- `<id>` is the `zeroc-ice/ice` issue number when the PR fixes a single public issue, and a short descriptive slug
  otherwise (no issue, several issues, or an issue tracked in a private repository — private issue numbers must
  never appear here).

The fragment content is the literal changelog bullet(s), inserted verbatim by the collation script:

- Each bullet is a Markdown list item starting with `-`, with continuation lines indented two spaces and wrapped at
  120 columns.
- Describe the change from the user's perspective, symptom first. The issue/PR reference stays out of the entry;
  git ties the fragment to its PR.
- A fragment may contain several bullets when one PR makes several user-visible changes.

A change with no user-facing impact gets **no fragment** — that is a deliberate decision, not an omission.

## Sections

| Directory | Changelog section |
| --------- | ----------------- |
| `general` | General Changes |
| `slice` | Slice Language Changes |
| `cpp` | C++ Changes |
| `csharp` | C# Changes |
| `java` | Java Changes |
| `js` | JavaScript Changes |
| `matlab` | MATLAB Changes |
| `objective-c` | Objective-C Changes |
| `php` | PHP Changes |
| `python` | Python Changes |
| `ruby` | Ruby Changes |
| `swift` | Swift Changes |
| `service-glacier2` | Ice Service Changes > Glacier2 |
| `service-windows` | Ice Service Changes > Ice Service installed as a Windows Service |
| `service-icebox` | Ice Service Changes > IceBox |
| `service-icegrid` | Ice Service Changes > IceGrid |
| `service-icestorm` | Ice Service Changes > IceStorm |
| `packaging` | Packaging Changes |

## Releasing

On the 3.7 branch, fragments come with the fixes backported from `main` (a cherry-pick carries its fragment along)
and with the fixes made on 3.7 only. They are collated when making a 3.7 release:

```shell
python3 scripts/collate_changelog.py <version>
```

The script inserts the collated sections into the `# Changes in Ice <version>` section of
`CHANGELOG-<major>.<minor>.md`, creating that section when it doesn't exist yet, and deletes the consumed fragments.
It fails when `CHANGELOG-<major>.<minor>.md` doesn't exist; use `--file` to collate into another file. It does not
touch git: review the diff, reorder entries as needed, regenerate the table of contents in your editor, and commit.
Use `--dry-run` to preview the collated sections.

`CHANGELOG-3.7.md` uses one heading level less than the changelogs of Ice 3.8 and later (`# Changes in Ice 3.7.12`,
`## C++ Changes`, `### IceGrid`), and the 3.7 version of the script generates headings to match. When the version
section already has entries, the collated sections follow them: merge the duplicate section headings by hand.

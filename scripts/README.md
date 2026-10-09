# Scripts

## generateIcePyDocs.py

Generates `python/modules/IcePy/DocStrings.h`, the docstring constants the IcePy C extension
ships, from `python/python/IcePy-stubs/__init__.pyi` — the single source of truth for IcePy
docstrings. Each stub declaration becomes one string constant: a signature line, a blank line,
and the prose.

Run it from anywhere to rewrite the header:

```shell
python3 scripts/generateIcePyDocs.py
```

`--check` regenerates the header to a temporary file, diffs it against the committed one, and
exits non-zero with the diff when the committed header is stale. CI runs this mode; after editing
the stub, regenerate the header and commit both files.

#!/usr/bin/env python3

# Copyright (c) ZeroC, Inc.

"""
Check that the IcePy stub agrees with the IcePy module.

IcePy is a C extension, so its documentation has to exist twice: Sphinx reads the docstrings the
module ships, and pyright and the IDEs read IcePy-stubs/__init__.pyi. Nothing links the two, so they
drift silently. This compares them and reports where they disagree.

The signature is compared as well as the prose. autodoc cannot introspect a C extension -- it reads
the first line of each docstring -- so that line is a signature the stub also spells out, and it can
drift just as easily.

Run it from the repository root after building Ice for Python:

    PYTHONPATH=python/python python3 scripts/checkIcePyStub.py
"""

from __future__ import annotations

import ast
import copy
import difflib
import sys
from pathlib import Path

STUB = Path(__file__).parents[1] / "python" / "python" / "IcePy-stubs" / "__init__.pyi"


def signatureOf(node: ast.FunctionDef | ast.AsyncFunctionDef) -> str:
    """Render a stub def the way the first line of a C docstring spells it."""
    args = copy.deepcopy(node.args)
    if args.posonlyargs and args.posonlyargs[0].arg in ("self", "cls"):
        del args.posonlyargs[0]
    elif args.args and args.args[0].arg in ("self", "cls"):
        del args.args[0]
    rendered = f"{node.name}({ast.unparse(args)})"
    return f"{rendered} -> {ast.unparse(node.returns)}" if node.returns else rendered


def stubEntries() -> dict[str, tuple[str | None, str | None]]:
    """Map each name in the stub to its (docstring, signature). Classes have no signature."""
    entries: dict[str, tuple[str | None, str | None]] = {}
    for node in ast.parse(STUB.read_text(encoding="utf-8")).body:
        if isinstance(node, ast.ClassDef):
            entries[node.name] = (ast.get_docstring(node), None)
            for member in node.body:
                if isinstance(member, (ast.FunctionDef, ast.AsyncFunctionDef)):
                    entries[f"{node.name}.{member.name}"] = (ast.get_docstring(member), signatureOf(member))
        elif isinstance(node, (ast.FunctionDef, ast.AsyncFunctionDef)):
            entries[node.name] = (ast.get_docstring(node), signatureOf(node))
    return entries


def shipped(name: str) -> tuple[bool, str | None]:
    """Return whether IcePy defines name, and the docstring it ships for it."""
    import IcePy

    obj: object = IcePy
    for part in name.split("."):
        if not hasattr(obj, part):
            return False, None
        obj = getattr(obj, part)
    return True, getattr(obj, "__doc__", None)


def split(doc: str | None, name: str) -> tuple[str | None, str]:
    """
    Separate a shipped docstring into its signature line and its prose.

    The line only counts as a signature if it opens with the name being documented. Python supplies
    its own docstring for an undocumented dunder -- "Return hash(self)." -- which otherwise looks
    close enough to one to be mistaken for it.
    """
    if not doc:
        return None, ""
    lines = doc.split("\n")
    if lines and lines[0].startswith(f"{name.split('.')[-1]}("):
        return lines[0].strip(), "\n".join(lines[1:]).strip()
    return None, doc.strip()


def isDunder(name: str) -> bool:
    """Python writes its own docstring for a dunder it generates, so those carry no signature."""
    leaf = name.rsplit(".", maxsplit=1)[-1]
    return leaf.startswith("__") and leaf.endswith("__")


def normalize(signature: str) -> str:
    """Ignore spacing around default values: the stub is formatted by ast, the docstring by hand."""
    return signature.replace(" = ", "=")


def main() -> int:
    problems: list[str] = []
    for name, (stubDoc, stubSignature) in sorted(stubEntries().items()):
        defined, shippedDoc = shipped(name)
        if not defined:
            # A private helper the stub declares for pyright need not be an attribute of the module,
            # but a public one going missing is the drift this is looking for.
            if not name.rsplit(".", maxsplit=1)[-1].startswith("_"):
                problems.append(f"{name}: declared in the stub, but IcePy does not define it")
            continue

        signature, prose = split(shippedDoc, name)

        if stubDoc and not prose:
            problems.append(f"{name}: documented in the stub, but IcePy ships no description")
        elif stubDoc and prose and stubDoc.strip() != prose:
            diff = "\n".join(
                f"    {line}"
                for line in difflib.unified_diff(
                    stubDoc.strip().splitlines(), prose.splitlines(), "stub", "IcePy", lineterm=""
                )
            )
            problems.append(f"{name}: descriptions differ\n{diff}")

        if stubSignature and not signature and not isDunder(name):
            problems.append(
                f"{name}: the stub gives a signature, but IcePy's docstring opens with no matching one."
                "\n    Sphinx reads the signature from that line, so it has to be there."
            )
        elif stubSignature and signature and normalize(stubSignature) != normalize(signature):
            problems.append(f"{name}: signatures differ\n    stub:  {stubSignature}\n    IcePy: {signature}")

    if problems:
        print(f"{len(problems)} difference(s) between the IcePy stub and the IcePy module:\n", file=sys.stderr)
        for p in problems:
            print(f"  {p}", file=sys.stderr)
        print(
            "\nBoth are hand-written and have to say the same thing: Sphinx reads the module's"
            "\ndocstrings, pyright and the IDEs read the stub.",
            file=sys.stderr,
        )
        return 1

    print("the IcePy stub matches the IcePy module")
    return 0


if __name__ == "__main__":
    sys.exit(main())

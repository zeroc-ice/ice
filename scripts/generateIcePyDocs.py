#!/usr/bin/env python3

# Copyright (c) ZeroC, Inc.

"""
Generate the IcePy docstring header from the IcePy stub.

The stub, python/python/IcePy-stubs/__init__.pyi, is the single source of truth for IcePy
docstrings. This script renders each stub declaration as the docstring the C extension ships --
a signature line, a blank line, and the prose, following the conventions the module already uses --
and writes them as string constants to python/modules/IcePy/DocStrings.h for the C++ sources to
reference.

Run it from anywhere; paths are resolved relative to this file:

    python3 scripts/generateIcePyDocs.py            # rewrite the header
    python3 scripts/generateIcePyDocs.py --check    # exit 1 with a diff if the header is stale
"""

import ast
import difflib
import inspect
import sys
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
STUB = ROOT / "python" / "python" / "IcePy-stubs" / "__init__.pyi"
HEADER = ROOT / "python" / "modules" / "IcePy" / "DocStrings.h"

# Classes whose tp_doc is the constructor signature line: a C type documents __init__ on the class,
# spelled with the class's name and the positional-only marker, because that is how it is called.
CTOR_CLASSES = {"Communicator", "ObjectPrx", "Operation", "Properties"}


def renderParams(fn: ast.FunctionDef, dropSelf: bool) -> str:
    """Render a def's parameter list, keeping the positional-only ``/`` marker where the stub
    declares one: Sphinx renders the line verbatim, so the marker is part of the signature."""
    args = fn.args
    posonly = list(args.posonlyargs)
    regular = list(args.args)
    if dropSelf:
        if posonly and posonly[0].arg == "self":
            posonly = posonly[1:]
        elif regular and regular[0].arg == "self":
            regular = regular[1:]
    params = posonly + regular
    defaults = [None] * (len(params) - len(args.defaults)) + list(args.defaults)
    parts = []
    for param, default in zip(params, defaults):
        part = param.arg
        if param.annotation is not None:
            part += ": " + ast.unparse(param.annotation)
        if default is not None:
            part += (" = " if param.annotation is not None else "=") + ast.unparse(default)
        parts.append(part)
    if posonly:
        parts.insert(len(posonly), "/")
    return ", ".join(parts)


def signatureLine(fn: ast.FunctionDef, name: str, dropSelf: bool) -> str:
    """Render a def as the signature line of its docstring. A def with no return annotation gets
    no ``->`` suffix."""
    line = f"{name}({renderParams(fn, dropSelf)})"
    if fn.returns is not None:
        line += " -> " + ast.unparse(fn.returns)
    return line


def isStatic(fn: ast.FunctionDef) -> bool:
    return any(isinstance(d, ast.Name) and d.id == "staticmethod" for d in fn.decorator_list)


def functionDoc(fns: list[ast.FunctionDef], dropSelf: bool) -> str:
    """Render a def -- or an @overload set, which stacks one signature line per overload and takes
    its prose from the last def -- as a complete docstring."""
    sigs = "\n".join(signatureLine(fn, fns[0].name, dropSelf) for fn in fns)
    doc = ast.get_docstring(fns[-1], clean=True)
    return sigs + "\n\n" + doc if doc else sigs


def groupDefs(body: list[ast.stmt]) -> dict[str, list[ast.FunctionDef]]:
    """Group defs by name, preserving first-occurrence order, so an @overload set renders once."""
    groups: dict[str, list[ast.FunctionDef]] = {}
    for node in body:
        if isinstance(node, ast.FunctionDef):
            groups.setdefault(node.name, []).append(node)
    return groups


def attributeDocs(body: list[ast.stmt]) -> dict[str, str]:
    """An annotated attribute followed by a string literal: the getset doc is that string,
    unwrapped to a single physical line (the stub wraps it to the line-length limit; the C source
    does not)."""
    docs = {}
    for prev, node in zip(body, body[1:]):
        if (
            isinstance(prev, ast.AnnAssign)
            and isinstance(prev.target, ast.Name)
            and isinstance(node, ast.Expr)
            and isinstance(node.value, ast.Constant)
            and isinstance(node.value.value, str)
        ):
            docs[prev.target.id] = " ".join(inspect.cleandoc(node.value.value).split("\n"))
    return docs


def classDoc(node: ast.ClassDef) -> str:
    doc = ast.get_docstring(node, clean=True)
    if doc:
        return doc
    if node.name in CTOR_CLASSES:
        init = groupDefs(node.body)["__init__"][0]
        return signatureLine(init, node.name, dropSelf=True)
    return f"IcePy.{node.name}"


def collect(tree: ast.Module) -> list[tuple[str, str, str, str]]:
    """Collect every docstring constant as (python name, constant name, kind, text), in stub
    source order. Dunders are skipped: those slots carry CPython-supplied docstrings. So are the
    typing helpers and the _t_* constants, which have no docstrings to carry."""
    entries = [("IcePy", "IcePy_DOC_module", "moduledoc", ast.get_docstring(tree, clean=True))]
    moduleGroups = groupDefs(tree.body)
    emittedFunctions = set()
    for node in tree.body:
        if isinstance(node, ast.FunctionDef) and node.name not in emittedFunctions:
            emittedFunctions.add(node.name)
            doc = functionDoc(moduleGroups[node.name], dropSelf=False)
            entries.append((f"IcePy.{node.name}", f"IcePy_DOC_{node.name}", "modulefunc", doc))
        elif isinstance(node, ast.ClassDef):
            entries.append((f"IcePy.{node.name}", f"IcePy_DOC_{node.name}", "tpdoc", classDoc(node)))
            groups = groupDefs(node.body)
            attrs = attributeDocs(node.body)
            emittedMethods = set()
            for member in node.body:
                if isinstance(member, ast.FunctionDef) and not member.name.startswith("__"):
                    if member.name not in emittedMethods:
                        emittedMethods.add(member.name)
                        fns = groups[member.name]
                        doc = functionDoc(fns, dropSelf=not isStatic(fns[0]))
                        entries.append(
                            (f"IcePy.{node.name}.{member.name}", f"IcePy_DOC_{node.name}_{member.name}", "method", doc)
                        )
                elif (
                    isinstance(member, ast.AnnAssign)
                    and isinstance(member.target, ast.Name)
                    and member.target.id in attrs
                ):
                    entries.append(
                        (
                            f"IcePy.{node.name}.{member.target.id}",
                            f"IcePy_DOC_{node.name}_{member.target.id}",
                            "getset",
                            attrs[member.target.id],
                        )
                    )
    constants = [entry[1] for entry in entries]
    duplicates = {c for c in constants if constants.count(c) > 1}
    if duplicates:
        sys.exit(f"error: duplicate constants: {', '.join(sorted(duplicates))}")
    return entries


def literal(text: str) -> str:
    """Render text as a C++ string literal: a plain quoted literal for a single line, a raw string
    literal with a non-colliding delimiter for multiple lines."""
    if "\n" not in text:
        return '"' + text.replace("\\", "\\\\").replace('"', '\\"') + '"'
    delimiter = "doc"
    while f'){delimiter}"' in text:
        delimiter += "_"
    return f'R"{delimiter}({text}){delimiter}"'


def render(entries: list[tuple[str, str, str, str]]) -> str:
    lines = [
        "// Copyright (c) ZeroC, Inc.",
        "",
        "// Generated by scripts/generateIcePyDocs.py from python/python/IcePy-stubs/__init__.pyi. Do not edit.",
        "",
        "#ifndef ICEPY_DOC_STRINGS_H",
        "#define ICEPY_DOC_STRINGS_H",
        "",
        "// clang-format off",
    ]
    for _, constant, _, text in entries:
        lines.append("")
        lines.append(f"inline constexpr const char* {constant} = {literal(text)};")
    lines += ["", "// clang-format on", "", "#endif", ""]
    return "\n".join(lines)


def main() -> None:
    if len(sys.argv) > 2 or (len(sys.argv) == 2 and sys.argv[1] != "--check"):
        sys.exit(f"usage: {sys.argv[0]} [--check]")

    tree = ast.parse(STUB.read_text(encoding="utf-8"))
    if ast.get_docstring(tree) is None:
        sys.exit(f"error: {STUB} has no module docstring")
    content = render(collect(tree))

    if len(sys.argv) == 2:
        committed = HEADER.read_text(encoding="utf-8") if HEADER.exists() else ""
        if content == committed:
            return
        with tempfile.NamedTemporaryFile("w", suffix=".h", delete=False) as generated:
            generated.write(content)
        diff = difflib.unified_diff(
            committed.splitlines(keepends=True),
            content.splitlines(keepends=True),
            fromfile=str(HEADER.relative_to(ROOT)),
            tofile=generated.name,
        )
        sys.stdout.writelines(diff)
        sys.exit(f"error: {HEADER.relative_to(ROOT)} is stale; run scripts/generateIcePyDocs.py")

    HEADER.write_text(content, encoding="utf-8")


if __name__ == "__main__":
    main()

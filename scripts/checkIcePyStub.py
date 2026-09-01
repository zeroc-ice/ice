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

Both directions are checked: every stub declaration against what the module ships, and every public
name the module defines against the stub, since a member the stub omits makes the type checker
reject a legitimate call.

Run it from the repository root after building Ice for Python:

    PYTHONPATH=python/python python3 scripts/checkIcePyStub.py
"""

from __future__ import annotations

import ast
import copy
import difflib
import inspect
import re
import sys
import types
from pathlib import Path

STUB = Path(__file__).parents[1] / "python" / "python" / "IcePy-stubs" / "__init__.pyi"

# Bookkeeping every type carries; nothing for a stub to declare. Dunders that vars(object) lists are
# skipped the same way, which also deliberately leaves the __hash__ = None of an unhashable type
# undeclared -- what a stub must declare is a slot object does not have, like ExecutorCall's __call__.
TYPE_METADATA = {"__dict__", "__weakref__", "__module__", "__qualname__", "__new__", "__slots__"}
LIST_ITEM = re.compile(r"(?:[-+*]|#\.|\d+[.)])\s")
FIELD_LIST_ITEM = re.compile(r":[^:\s][^:]*:\s")
NUMPYDOC_FIELD = re.compile(r"[^:]+\s:\s\S")


def signatureOf(node: ast.FunctionDef | ast.AsyncFunctionDef) -> str:
    """Render a stub def the way the first line of a C docstring spells it."""
    args = copy.deepcopy(node.args)
    if args.posonlyargs and args.posonlyargs[0].arg in ("self", "cls"):
        del args.posonlyargs[0]
    elif args.args and args.args[0].arg in ("self", "cls"):
        del args.args[0]
    rendered = f"{node.name}({ast.unparse(args)})"
    return f"{rendered} -> {ast.unparse(node.returns)}" if node.returns else rendered


def isProperty(node: ast.FunctionDef | ast.AsyncFunctionDef) -> bool:
    """Return whether node declares a property rather than a callable method."""
    return any(
        isinstance(decorator, ast.Name)
        and decorator.id == "property"
        or isinstance(decorator, ast.Attribute)
        and decorator.attr == "property"
        for decorator in node.decorator_list
    )


def attributeDoc(body: list[ast.stmt], index: int) -> str | None:
    """Return the docstring of the annotated attribute at index: the string literal right below it."""
    if index + 1 < len(body):
        after = body[index + 1]
        if isinstance(after, ast.Expr) and isinstance(after.value, ast.Constant) and isinstance(after.value.value, str):
            return inspect.cleandoc(after.value.value)
    return None


def stubDeclarations() -> tuple[
    dict[str, tuple[str | None, str | None, bool]], list[str], dict[str, set[str]], set[str]
]:
    """
    Parse the stub into (entries, duplicates, classMembers, topLevel).

    entries maps each documented name to its (docstring, signature, value). Classes, properties and
    attributes carry no signature, and value marks the properties and attributes: IcePy must not
    document those as callables either, or the stub promises an attribute for something the caller
    has to call. duplicates lists the names entries could not keep apart: repeated defs -- an
    @overload set, say -- silently overwrite each other, so they are reported instead of
    half-compared.
    classMembers maps each class to every member it declares, its stub base classes included, and
    topLevel holds every name the stub declares at module scope; both exist for the reverse sweep.
    """
    entries: dict[str, tuple[str | None, str | None, bool]] = {}
    duplicates: list[str] = []
    ownMembers: dict[str, set[str]] = {}
    bases: dict[str, list[str]] = {}
    topLevel: set[str] = set()

    def add(name: str, doc: str | None, signature: str | None, value: bool = False) -> None:
        if name in entries:
            duplicates.append(name)
            del entries[name]
        elif name not in duplicates:
            entries[name] = (doc, signature, value)

    for node in ast.parse(STUB.read_text(encoding="utf-8")).body:
        if isinstance(node, ast.ClassDef):
            topLevel.add(node.name)
            add(node.name, ast.get_docstring(node), None)
            members = ownMembers[node.name] = set()
            bases[node.name] = [base.id for base in node.bases if isinstance(base, ast.Name)]
            for index, member in enumerate(node.body):
                if isinstance(member, (ast.FunctionDef, ast.AsyncFunctionDef)):
                    members.add(member.name)
                    declaresProperty = isProperty(member)
                    add(
                        f"{node.name}.{member.name}",
                        ast.get_docstring(member),
                        None if declaresProperty else signatureOf(member),
                        declaresProperty,
                    )
                elif isinstance(member, ast.AnnAssign) and isinstance(member.target, ast.Name):
                    members.add(member.target.id)
                    add(f"{node.name}.{member.target.id}", attributeDoc(node.body, index), None, True)
        elif isinstance(node, (ast.FunctionDef, ast.AsyncFunctionDef)):
            topLevel.add(node.name)
            add(node.name, ast.get_docstring(node), signatureOf(node))
        elif isinstance(node, ast.AnnAssign) and isinstance(node.target, ast.Name):
            topLevel.add(node.target.id)
        elif isinstance(node, ast.Assign):
            topLevel.update(target.id for target in node.targets if isinstance(target, ast.Name))

    def membersOf(className: str) -> set[str]:
        members = set(ownMembers.get(className, ()))
        for base in bases.get(className, ()):
            members |= membersOf(base)
        return members

    return entries, duplicates, {name: membersOf(name) for name in ownMembers}, topLevel


def shipped(name: str) -> tuple[bool, str | None, bool]:
    """
    Return whether IcePy defines name, the docstring it ships for it, and whether that docstring is
    CPython's rather than IcePy's.

    A slot wrapper (tp_init, tp_richcompare and friends), a member inherited from object, and the
    __hash__ = None of an unhashable type all carry documentation CPython generates -- there is no
    hand-written copy behind them for the stub to agree with. Members are resolved through the
    class's own MRO because getattr on a class also consults the metaclass, which gives every class
    a __call__ its instances do not have.
    """
    import IcePy

    head, _, member = name.partition(".")
    missing = object()
    obj = vars(IcePy).get(head, missing)
    if obj is missing:
        return False, None, False
    if not member:
        doc = obj.__doc__
        if isinstance(obj, type) and doc == f"IcePy.{head}":
            # The C sources set tp_doc to the type's own name as a placeholder for "undocumented".
            doc = None
        return True, doc, False
    if not isinstance(obj, type):
        return False, None, False
    for cls in obj.__mro__:
        if member in vars(cls):
            value = vars(cls)[member]
            if isinstance(value, (staticmethod, classmethod)):
                value = value.__func__  # the wrapper's own __doc__ describes staticmethod itself
            pythonSupplied = (
                value is None
                and member == "__hash__"
                or isinstance(value, types.WrapperDescriptorType)
                or cls is object
            )
            return True, None if value is None else getattr(value, "__doc__", None), pythonSupplied
    return False, None, False


def split(doc: str | None, name: str) -> tuple[str | None, str]:
    """
    Separate a shipped docstring into its signature line and its prose.

    The line only counts as a signature if it opens with the name being documented -- an attribute's
    description, "bool: ...", must not be mistaken for one.
    """
    if not doc:
        return None, ""
    lines = doc.split("\n")
    if lines and lines[0].startswith(f"{name.split('.')[-1]}("):
        return lines[0].strip(), "\n".join(lines[1:]).strip()
    return None, doc.strip()


def normalizeSignature(signature: str) -> str:
    """Ignore spacing around default values: the stub is formatted by ast, the docstring by hand."""
    return signature.replace(" = ", "=")


def normalizeProse(text: str) -> str:
    """
    Fold hard line wrapping so reflowing a paragraph does not read as drift.

    Consecutive prose lines with the same indentation join into one. Structural lines such as list
    items, fields, directives, and section underlines remain separate, as do indented literal and
    preformatted blocks.

    A marker only makes a line structural where reST would begin a block: after a blank line, or at
    a new indentation. reST wants that blank line before a list, a field list, or a directive, so a
    wrapped line that merely opens with a marker -- "30901. For pre-releases..." -- is the paragraph
    continuing, not an enumerated item. A section underline is the exception: it attaches to the
    line right above it.
    """
    out: list[str] = []
    joinIndent = None  # indentation of the line out[-1] belongs to, when it can accept continuations
    literalMarkerIndent = None
    preformattedIndent = None
    for line in text.split("\n"):
        stripped = line.strip()
        indent = len(line) - len(line.lstrip())

        if preformattedIndent is not None:
            if not stripped or indent >= preformattedIndent:
                out.append(line)
                joinIndent = None
                continue
            preformattedIndent = None

        if literalMarkerIndent is not None:
            if not stripped:
                out.append(line)
                joinIndent = None
                continue
            if indent > literalMarkerIndent:
                preformattedIndent = indent
                out.append(line)
                joinIndent = None
                continue
            literalMarkerIndent = None

        if not stripped:
            out.append(line)
            joinIndent = None
            continue

        continuation = indent == joinIndent
        underline = not stripped.strip("-=~^\"'`#*+_")
        structural = underline or (
            not continuation
            and (
                LIST_ITEM.match(stripped) is not None
                or FIELD_LIST_ITEM.match(stripped) is not None
                or NUMPYDOC_FIELD.match(stripped) is not None
                or stripped.startswith((".. ", ">>>", "...", "```", "|"))
            )
        )
        if not structural and continuation:
            out[-1] = f"{out[-1]} {stripped}"
        else:
            out.append(line)
        joinIndent = None if structural else indent
        if stripped.endswith("::"):
            literalMarkerIndent = indent
            joinIndent = None
    return "\n".join(out)


def reverseProblems(classMembers: dict[str, set[str]], topLevel: set[str]) -> list[str]:
    """The stub-driven walk cannot see a member the stub omits; sweep the module's own inventory."""
    import IcePy

    problems: list[str] = []
    for name in sorted(vars(IcePy)):
        if not name.startswith("_") and name not in topLevel:
            problems.append(f"{name}: IcePy defines it, but the stub does not declare it")
    for className, members in sorted(classMembers.items()):
        cls = vars(IcePy).get(className)
        if not isinstance(cls, type):
            continue  # a class IcePy does not define is the stub-driven walk's finding
        for member in sorted(vars(cls)):
            if member in members or member.startswith("_") and not member.endswith("__"):
                continue
            if member.startswith("__") and (member in vars(object) or member in TYPE_METADATA):
                continue
            problems.append(f"{className}.{member}: IcePy defines it, but the stub does not declare it")
    return problems


def main() -> int:
    problems: list[str] = []
    entries, duplicates, classMembers, topLevel = stubDeclarations()

    for name in duplicates:
        problems.append(
            f"{name}: the stub declares it more than once; this check compares single definitions"
            " only, so teach it about @overload first"
        )

    for name, (stubDoc, stubSignature, stubValue) in sorted(entries.items()):
        defined, shippedDoc, pythonSupplied = shipped(name)
        if not defined:
            # A private helper the stub declares for pyright need not be an attribute of the module,
            # but a public one going missing is the drift this is looking for.
            if not name.rsplit(".", maxsplit=1)[-1].startswith("_"):
                problems.append(f"{name}: declared in the stub, but IcePy does not define it")
            continue
        if pythonSupplied:
            continue

        signature, prose = split(shippedDoc, name)

        if stubDoc and not prose:
            problems.append(f"{name}: documented in the stub, but IcePy ships no description")
        elif prose and not stubDoc:
            problems.append(f"{name}: IcePy ships a description, but the stub does not document it")
        elif stubDoc and prose:
            normalizedStubDoc = normalizeProse(stubDoc.strip())
            normalizedShippedDoc = normalizeProse(prose)
            if normalizedStubDoc != normalizedShippedDoc:
                diff = "\n".join(
                    f"    {line}"
                    for line in difflib.unified_diff(
                        normalizedStubDoc.splitlines(),
                        normalizedShippedDoc.splitlines(),
                        "stub",
                        "IcePy",
                        lineterm="",
                    )
                )
                problems.append(f"{name}: descriptions differ\n{diff}")

        if stubValue and signature:
            problems.append(
                f"{name}: the stub declares it as a value, but IcePy documents it as callable.\n    IcePy: {signature}"
            )
        elif stubSignature and not signature:
            problems.append(
                f"{name}: the stub gives a signature, but IcePy's docstring opens with no matching one."
                "\n    Sphinx reads the signature from that line, so it has to be there."
            )
        elif stubSignature and signature and normalizeSignature(stubSignature) != normalizeSignature(signature):
            problems.append(f"{name}: signatures differ\n    stub:  {stubSignature}\n    IcePy: {signature}")

    problems += reverseProblems(classMembers, topLevel)

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

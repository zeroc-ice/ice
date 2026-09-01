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
# A section underline, or any other adornment: reST spells one as a single punctuation character
# repeated, so recognizing the shape beats naming the characters one by one and missing "::::::".
ADORNMENT = re.compile(r"([!-/:-@\[-`{-~])\1+")
FIELD_LIST_ITEM = re.compile(r":[^:\s][^:]*:\s")
NUMPYDOC_FIELD = re.compile(r"[^:]+\s:\s\S")
# A stub-only construct: it exists for the type checker and is not an attribute of the module.
TYPING_HELPERS = {"TypeVar", "ParamSpec", "TypeVarTuple", "NewType", "TypeAliasType"}


def signatureOf(node: ast.FunctionDef | ast.AsyncFunctionDef, name: str | None = None) -> str:
    """
    Render a stub def the way the first line of a C docstring spells it.

    name replaces the def's own, which is what a constructor needs: a C type documents __init__ on
    the class, spelled with the class's name, because that is how it is called.
    """
    args = copy.deepcopy(node.args)
    if args.posonlyargs and args.posonlyargs[0].arg in ("self", "cls"):
        del args.posonlyargs[0]
    elif args.args and args.args[0].arg in ("self", "cls"):
        del args.args[0]
    rendered = f"{name or node.name}({ast.unparse(args)})"
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
    dict[str, tuple[str | None, str | None, str | None]], list[str], dict[str, set[str]], set[str]
]:
    """
    Parse the stub into (entries, duplicates, classMembers, topLevel).

    entries maps each documented name to its (docstring, signature, type). Classes, properties and
    attributes carry no signature, and type is the one a property or an attribute declares: IcePy
    must not document those as callables, and its description opens by naming the same type.
    Everything the stub declares at module scope goes in too, so that a name the stub invents is
    caught. duplicates lists the names entries could not keep apart: repeated defs -- an @overload
    set, say -- silently overwrite each other, so they are reported instead of half-compared.
    classMembers maps each class to every member it declares, its stub base classes included, and
    topLevel holds every name the stub declares at module scope; both exist for the reverse sweep.
    """
    entries: dict[str, tuple[str | None, str | None, str | None]] = {}
    duplicates: list[str] = []
    ownMembers: dict[str, set[str]] = {}
    bases: dict[str, list[str]] = {}
    topLevel: set[str] = set()

    def add(name: str, doc: str | None, signature: str | None, declared: str | None = None) -> None:
        if name in entries:
            duplicates.append(name)
            del entries[name]
        elif name not in duplicates:
            entries[name] = (doc, signature, declared)

    for node in ast.parse(STUB.read_text(encoding="utf-8")).body:
        if isinstance(node, ast.ClassDef):
            topLevel.add(node.name)
            # __init__ itself is a slot, and the wrapper CPython puts there documents object's
            # generic one. The signature the stub declares for it lives on the class, which is where
            # a C type spells its constructor and the only place the module can be held to it.
            initializer = next(
                (m for m in node.body if isinstance(m, ast.FunctionDef) and m.name == "__init__"),
                None,
            )
            add(
                node.name,
                ast.get_docstring(node),
                signatureOf(initializer, node.name) if initializer else None,
            )
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
                        ast.unparse(member.returns) if declaresProperty and member.returns else None,
                    )
                elif isinstance(member, ast.AnnAssign) and isinstance(member.target, ast.Name):
                    members.add(member.target.id)
                    add(
                        f"{node.name}.{member.target.id}",
                        attributeDoc(node.body, index),
                        None,
                        ast.unparse(member.annotation),
                    )
        elif isinstance(node, (ast.FunctionDef, ast.AsyncFunctionDef)):
            topLevel.add(node.name)
            add(node.name, ast.get_docstring(node), signatureOf(node))
        elif isinstance(node, ast.AnnAssign) and isinstance(node.target, ast.Name):
            topLevel.add(node.target.id)
            add(node.target.id, None, None, ast.unparse(node.annotation))
        elif isinstance(node, ast.Assign):
            # A TypeVar and its kind exist for the type checker alone; every other module-scope name
            # is one IcePy has to define, and going through add() is what gets that checked.
            call = node.value if isinstance(node.value, ast.Call) else None
            helper = call is not None and isinstance(call.func, ast.Name) and call.func.id in TYPING_HELPERS
            for target in node.targets:
                if isinstance(target, ast.Name):
                    topLevel.add(target.id)
                    if not helper:
                        add(target.id, None, None)

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
        if isinstance(obj, type):
            if doc == f"IcePy.{head}":
                # The C sources set tp_doc to the type's own name as a placeholder for "undocumented".
                doc = None
        elif doc is not None and doc == getattr(type(obj), "__doc__", None):
            # A plain value carries no docstring of its own: __doc__ falls through to its class --
            # a fresh string each time, for a C type -- and the class's description is not a
            # description of this module attribute.
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


def normalizeType(declared: str) -> str:
    """
    Ignore the module a type is named through, and the spacing inside it.

    Ice re-exports IcePy's C types unchanged -- Ice.EndpointInfo is IcePy.EndpointInfo -- so the two
    qualified spellings and the bare one all name the same class, and the C sources use both.
    """
    return re.sub(r"\b(?:Ice|IcePy)\.", "", declared).replace(" ", "")


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

    A list item's own wrapping folds too. Its text continues on the following lines, indented past
    the marker, so the first of those opens a paragraph that the rest join -- rewrapping a bullet is
    no more a change than rewrapping anything else. That first line is still read as a marker if it
    is one, which is what a nested list looks like. A field list and a directive are left alone:
    what follows them is an indented body, not the same sentence carrying on.
    """
    out: list[str] = []
    joinIndent = None  # indentation of the line out[-1] belongs to, when it can accept continuations
    itemIndent = None  # indentation of a list item whose text may still continue, indented past it
    literalMarkerIndent = None
    preformattedIndent = None
    for line in text.split("\n"):
        stripped = line.strip()
        indent = len(line) - len(line.lstrip())

        if preformattedIndent is not None:
            if not stripped or indent >= preformattedIndent:
                out.append(line)
                joinIndent = itemIndent = None
                continue
            preformattedIndent = None

        if literalMarkerIndent is not None:
            if not stripped:
                out.append(line)
                joinIndent = itemIndent = None
                continue
            if indent > literalMarkerIndent:
                preformattedIndent = indent
                out.append(line)
                joinIndent = itemIndent = None
                continue
            literalMarkerIndent = None

        if not stripped:
            out.append(line)
            joinIndent = itemIndent = None
            continue

        continuation = indent == joinIndent
        listItem = LIST_ITEM.match(stripped) is not None
        underline = ADORNMENT.fullmatch(stripped) is not None
        structural = underline or (
            not continuation
            and (
                listItem
                or FIELD_LIST_ITEM.match(stripped) is not None
                or NUMPYDOC_FIELD.match(stripped) is not None
                or stripped.startswith((".. ", ">>>", "...", "```", "|"))
            )
        )
        if not structural and (continuation or (itemIndent is not None and indent > itemIndent)):
            out[-1] = f"{out[-1]} {stripped}"
        else:
            out.append(line)
        joinIndent = None if structural else indent
        itemIndent = indent if structural and listItem and not underline else None
        if stripped.endswith("::"):
            literalMarkerIndent = indent
            joinIndent = itemIndent = None
    return "\n".join(out)


def reverseProblems(classMembers: dict[str, set[str]], topLevel: set[str]) -> list[str]:
    """The stub-driven walk cannot see a member the stub omits; sweep the module's own inventory."""
    import IcePy

    problems: list[str] = []
    for name, value in sorted(vars(IcePy).items()):
        if name.startswith("_"):
            continue
        if name not in topLevel:
            problems.append(f"{name}: IcePy defines it, but the stub does not declare it")
        elif isinstance(value, type) and name not in classMembers:
            # Declaring the name is not enough: `Logger: Any` in place of `class Logger:` type-checks
            # anything, and takes every member of the class out of the comparison with it.
            problems.append(f"{name}: IcePy defines a class, but the stub declares it as a value")
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

    for name, (stubDoc, stubSignature, stubType) in sorted(entries.items()):
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

        if stubType is not None and signature:
            problems.append(
                f"{name}: the stub declares it as a value, but IcePy documents it as callable.\n    IcePy: {signature}"
            )
        elif stubType is not None and prose:
            # The C sources open a data member's description by naming its type -- "bool: Specifies
            # whether ..." -- which is the only thing the annotation can be checked against.
            documented, named, _ = prose.split("\n", maxsplit=1)[0].partition(": ")
            if not named:
                problems.append(
                    f"{name}: IcePy's description does not open by naming the type, so the stub's is unchecked"
                )
            elif normalizeType(documented) != normalizeType(stubType):
                problems.append(
                    f"{name}: the stub's type and the one IcePy documents differ."
                    f"\n    stub:  {stubType}\n    IcePy: {documented}"
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

# Copyright (c) ZeroC, Inc.

import IcePy


def compileSlice(args: list[str]) -> int:
    """
    Compile Slice definitions into Python code using the Slice for Python compiler (`slice2py`).

    This function behaves exactly like the `slice2py` command-line tool: it processes the provided
    Slice definitions and generates Python files in the directory specified by the `--output-dir`
    argument.

    If you want to load Slice definitions directly into the current Python interpreter *without*
    generating files, use :func:`loadSlice` instead.

    Parameters
    ----------
    args : list[str]
        The list of arguments to pass to the Slice for Python compiler.
        The accepted arguments are identical to those supported by the `slice2py` command-line tool.

    Returns
    -------
    int
        The exit code returned by the Slice for Python compiler.
    """
    args.insert(0, "compileSlice")
    return IcePy.compileSlice(args)


def loadSlice(args: list[str]) -> None:
    """
    Load Slice definitions directly into the current Python environment.

    Unlike :func:`compileSlice`, this function does not generate any Python source files. Instead,
    the generated Python code is loaded directly into the running interpreter.

    Parameters
    ----------
    args : list[str]
        The list of arguments to pass to the Slice loader.
        The accepted arguments are the same as those supported by the `slice2py` tool, except that
        the `--output-dir`, `--depend`, `--depend-xml`, and `--depend-file` options are ignored.

    Raises
    ------
    RuntimeError
        If an error occurs during Slice parsing or compilation.

    Notes
    -----
    Calling :func:`loadSlice` multiple times replaces the previously generated type definitions.
    Any existing objects, proxies, or unmarshaled instances created from the earlier definitions
    become incompatible with the newly loaded types. As a result, attempts to marshal or unmarshal
    values using these stale instances may fail or raise exceptions, because their types no longer
    match the currently loaded definitions.
    """
    args.insert(0, "loadSlice")
    IcePy.loadSlice(args)


__all__ = ["compileSlice", "loadSlice"]

// Copyright (c) ZeroC, Inc.

#pragma once

// These doc-comments deliberately contain characters that are special inside a Python triple-quoted
// docstring: an embedded triple-quote sequence, and backslash escape sequences such as '\u'. The
// generated docstrings must escape them so that the generated code is valid Python.

["python:identifier:generated.test.Slice.docComments.Test"]
module Test
{
    /// A struct. Example: """triple""" quoted, path C:\users\name, escape é.
    /// @see I
    struct S
    {
        /// A field with """ in its \u description.
        int x;
    }

    /// An exception with """ embedded.
    exception E
    {
        /// A field \with a backslash.
        string reason;
    }

    /// An enum. Example: """value""".
    enum Color
    {
        /// Red with """ and \u escape.
        Red,
        Green
    }

    interface I
    {
        /// Does a thing. Example: """quoted""".
        /// @param v the value \u with a backslash and """ quotes.
        /// @return a result """ with quotes.
        /// @throws E when it fails \with backslash.
        int op(int v) throws E;
    }
}

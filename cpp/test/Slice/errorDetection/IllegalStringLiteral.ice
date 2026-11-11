// Copyright (c) ZeroC, Inc.

module Test
{
    // Control characters other than tabs, vertical tabs, and form feeds cannot appear unescaped in a string literal.
    // This file uses a raw 0x1F byte: the one control character the preprocessor passes through without a warning.
    const string unitSeparator = "a^_b";
}

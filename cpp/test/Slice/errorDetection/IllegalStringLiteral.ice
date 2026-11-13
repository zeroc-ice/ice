// Copyright (c) ZeroC, Inc.
// ice-fmt: ignore-file
module Test
{
    // Test that carriage returns are reported in string literals.
    const string carriageReturn = "ab";

    // Test that unsupported control characters are reported. We use 0x1F,
    // since it's the only control character mcpp passes through without a warning.
    const string unitSeparator = "ab";
}

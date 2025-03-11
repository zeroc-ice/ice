// Copyright (c) ZeroC, Inc.

// Duplicate metadata should be rejected.
[["cpp:header-ext:hh"]]
[["cpp:header-ext:hh"]]
[["cpp:source-ext:cc"]]
[["cpp:source-ext:cc"]]
[["cpp:dll-export:Test"]]
[["cpp:dll-export:Test"]]

// Duplicate metadata should be rejected, and this is missing a required argument.
[["cpp:header-ext"]]
[["cpp:header-ext:"]]
[["cpp:source-ext"]]
[["cpp:source-ext:"]]
[["cpp:dll-export"]]
[["cpp:dll-export:"]]

// It's okay for this metadata to appear multiple times, but it's missing a required argument.
[["cpp:include"]]
[["cpp:include:"]]

// Unknown file metadata is disallowed.
[["unknown"]]
[["cpp:unknown"]]
[["bad:unknown"]]
// Ignore metadata that has a valid language prefix, but that doesn't match the current compiler.
[["php:unknown"]]

// Passing an argument to metadata that doesn't take arguments is disallowed.
[["cpp:no-default-include:foo"]]

// The 'amd' metadata cannot be used as file-level metadata.
[["amd"]]

module Test
{
    // The 'cpp:header-ext' metadata cannot be used as local metadata. It also cannot take multiple arguments.
    ["cpp:header-ext:a, b, c"]
    sequence<byte> Blob;

    // Providing a message to 'deprecated' is optional, but it cannot be applied twice to the same thing.
    ["deprecated", "deprecated:do not use this"]
    class Container
    {
        // Unknown local metadata is disallowed.
        ["unknown", "cpp:unknown", "bad:unknown"]
        bool b;

        // Ignore metadata that has a valid language prefix, but that doesn't match the current compiler.
        ["php:unknown"]
        int i;

        // Passing an argument to metadata that doesn't take arguments is disallowed.
        ["cpp:array:foo"]
        Blob blob;
    }

    interface I
    {
        ["cpp:type:std::list<std::string>"]
        void op1();

        ["cpp:array"]
        void op2();

        void op3(["cpp:type:my_string"] string s);

        void op4(["cpp:view-type:my_string"] string s);

        // Certain metadata restrict what arguments are valid; 'something' is not a valid format.
        // It is also disallowed to pass multiple arguments to metadata which only expects one.
        // And this will also trigger a duplicate metadata warning. All 3 warnings should be emitted from this.
        ["format:something", "format:default, sliced"]
        void op5();
    }

    ["cpp:const"] ["cpp:ice_print"]
    struct S
    {
        int i;
    }

    ["cpp:virtual"]
    exception E
    {
    }

    ["bad", "cpp:nope", "java:bad"] // We skip metadata that has a valid (but inapplicable) language prefix.
    class C
    {
    }

    ["cpp98:foo", "cpp11:bar"] // We still warn for unknown language prefixes (cpp98 and cpp11 metadata were removed in 3.8)
    class P
    {
    }

    // Metadata is shared between forward declarations and definitions.
    // We allow duplicate metadata, but require that that metadata must be identical.

    ["java:nonsense", "deprecated:hello", "amd"]
    interface K;

    ["java:nonsense"] ["deprecated:goodbye"] ["amd"]
    interface K
    {
    }

    ["java:nonsense", "deprecated", "amd"]
    interface K;
}

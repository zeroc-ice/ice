// Copyright (c) ZeroC, Inc.

// Error: multiple commas in a row.
[["hello",,]]

// Warning: useless empty metadata.
[[]]

module MyModule
{
    // Error: metadata must be wrapped in strings.
    [cpp:type:string, "this:is:ok", deprecated]
    const int MyConst = 79;

    // Error: multiple commas in a row.
    ["hello",,]
    class TestClass
    {
        // Warning: useless empty metadata.
        []
        double temperature;
        double humidity;
    }
}

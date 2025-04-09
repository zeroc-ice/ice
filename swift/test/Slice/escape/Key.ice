// Copyright (c) ZeroC, Inc.

["swift:identifier:escapedImport"]
module import
{
    ["swift:identifier:`continue`"]
    enum continue
    {
        ["swift:identifier:myFirstEnumerator"] let,
        ["swift:identifier:mySecondEnumerator"] var
    }

    ["swift:identifier:`guard`"]
    struct guard
    {
        ["swift:identifier:`default`"]
        int default;
    }

    ["swift:identifier:`break`"]
    interface break
    {
        ["amd"] ["swift:identifier:`case`"] void case(
            ["swift:identifier:`class`"] int \class,
            ["swift:identifier:`try`"] out int try
        );
    }

    ["swift:identifier:`switch`"]
    class switch
    {
        ["swift:identifier:`remappedExport`"] break* export;
        ["swift:identifier:remappedVolatile"] int volatile;
    }

    ["swift:identifier:`return`"]
    exception return
    {
        // It should be fine to use built-in types as identifiers.
        int Int32;
    }

    ["swift:identifier:`as`"]
    exception as extends return
    {
        ["swift:identifier:`switch`"]
        int switch;
    }

    ["swift:identifier:`do`"]
    interface do extends break
    {
        ["swift:identifier:`public`"]
        void public();

        guard goto(
            ["swift:identifier:`if`"] continue if,
            guard d,
            ["swift:identifier:`private`"] switch private,
            do* mutable,
            ["swift:identifier:foo"] break* namespace,
            switch not,
            long or
        ) throws return, as;
    }

    ["swift:identifier:`fileprivate`"]
    sequence<guard> fileprivate;

    ["swift:identifier:`for`"]
    dictionary<string, guard> for;

    ["swift:identifier:`typealias`"]
    const int typealias = 0;
}

// Copyright (c) ZeroC, Inc.

#pragma once

[["suppress-warning:deprecated"]] // for 'cs:namespace' metadata

["cs:namespace:Ice.namespacemd"]
module WithNamespace
{
    class C1
    {
        int i;
    }

    class C2 extends C1
    {
        long l;
    }

    exception E1
    {
        int i;
    }

    exception E2 extends E1
    {
        long l;
    }

    // A module nested under a cs:namespace-tagged module: its types must map to
    // WithNamespace.Inner.*, not a doubled-prefix namespace (regression test for #5478).
    module Inner
    {
        struct S
        {
            int i;
        }
    }
}

["cs:namespace:ZeroC"]
module Foo
{
    module Bar
    {
        struct S
        {
            string str;
        }
    }
}

["cs:namespace:ZeroC"]
module Other
{
    module Bar
    {
        struct S
        {
            string str;
        }
    }

    module Baz
    {
        struct S
        {
            string str;
        }
    }
}

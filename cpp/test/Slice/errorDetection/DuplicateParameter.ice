//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

[["suppress-warning:deprecated"]] // for classes with operations

module Test
{

class Foo
{
    long l;
    string bar1(string s, int s);
    string bar2(string s, out int s);
    string bar3(out string s, out int s);
    string bar4(string s, int i, out int i);
}

interface IFoo
{
    void op();
    string bar1(string s, int s);
    string bar2(string s, out int s);
    string bar3(out string s, out int s);
    string bar4(string s, int i, out int i);
}

}

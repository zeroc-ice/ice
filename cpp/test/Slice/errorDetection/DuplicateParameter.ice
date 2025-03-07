// Copyright (c) ZeroC, Inc.

module Test
{
    interface IFoo
    {
        void op();
        string bar1(string s, int s);
        string bar2(string s, out int s);
        string bar3(out string s, out int s);
        string bar4(string s, int i, out int i);
    }
}

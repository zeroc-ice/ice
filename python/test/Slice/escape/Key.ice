// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

[["suppress-warning:deprecated"]]

module and
{
    enum assert
    {
        break
    }

    struct continue
    {
        int def;
    }

    interface del
    {
        ["amd"] void elif(int else, out int except);
    }

    interface exec
    {
        void finally();
    }

    class for
    {
        int lambda;
        exec* from;
        int global;
    }

    interface if extends exec, del
    {
    }

    sequence<assert> import;
    dictionary<string,assert> in;

    exception is
    {
        int lambda;
    }

    exception not extends is
    {
        int or;
        int pass;
    }

    local interface print
    {
        assert raise(continue else, for return, if try, del* while, exec* yield,
                     for* lambda, if* or, int global)
                    throws is;
    }

    const int lambda = 0;

    enum EnumNone
    {
        None
    }
}

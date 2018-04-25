// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

[["underscore", "suppress-warning:deprecated"]]

module BEGIN
{
    enum END
    {
        alias
    }

    struct and
    {
        int begin;
    }

    interface break
    {
        void case(int clone, int def);
        void to_a();
        void instance_variable_set();
        void instance_variables();
    }

    class display
    {
        int when;
        int do;
        break* dup;
        int else;
    }

    interface elsif extends break
    {
    }

    sequence<END> rescue;
    dictionary<string, END> ensure;

    exception next
    {
        int new;
    }

    exception nil extends next
    {
        int not;
        int or;
    }

    local interface extend
    {
        END for(display freeze, elsif hash, break* if, display inspect, elsif method, int methods)
            throws nil;
    }

    const int redo = 1;
}

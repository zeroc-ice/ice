// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

[["underscore", "suppress-warning:deprecated"]]

module and
{
    enum array
    {
        as
    }

    struct xor
    {
        int abstract;
        int clone;
        int private;
        int protected;
        int public;
        int this;
        int throw;
        int use;
        int var;
    }

    interface break
    {
        void case(int catch, out int try);
    }

    interface function
    {
        void continue(int declare, int default);
    }

    interface die
    {
        void do();
    }

    class echo
    {
        int if;
        int else;
        die* elseif;
        int empty;
    }

    interface enddeclare extends die, function
    {
    }
    sequence<array> endfor;
    dictionary<string,array> endforeach;

    exception endif
    {
        int endswitch;
    }

    exception endwhile extends endif
    {
        int eval;
        int exit;
    }

    local interface for
    {
        array foreach(break if, echo global, enddeclare require, function* include,
                      die* return, echo* isset, enddeclare* list, int new, int static) throws endif, endwhile;
    }

    const int or = 0;
    const int print = 0;
    const int require_once = 0;
}

// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

module and
{
    enum array
    {
        as
    };
    struct break
    {
        int case;
    };
    interface cfunction
    {
        void continue(int declare, int default);
    };
    interface die
    {
        void do();
    };
    class echo
    {
        void else(die* elseif, out int empty);
    };
    class enddeclare extends echo implements die, cfunction
    {
    };
    sequence<array> endfor;
    dictionary<string,array> endforeach;

    exception endif { int endswitch; };
    exception endwhile extends endif { int eval; int exit; };

    local interface for
    {
        array foreach(break if, echo global, enddeclare require, cfunction* include,
		      die* return, echo* isset, enddeclare* list, int new, int static)
            throws endif, endwhile;
    };

    const int or = 0;
    const int print = 0;
};

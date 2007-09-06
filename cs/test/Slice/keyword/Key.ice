// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

module abstract
{

enum as
{
    base
};

struct break
{
    int readonly;
};

interface case
{
    ["amd"] void catch(int checked, out int continue);
};

interface decimal
{
    void default();
};

class delegate
{
    ["ami"] void foo(case* else, out int event);
};

class explicit extends delegate implements decimal, case
{
};
["clr:collection"] sequence<as> extern;
dictionary<string, break> while;
["clr:collection"] dictionary<string, as> finally;

exception fixed
{
    int for;
};

exception foreach extends fixed
{
    int goto;
    int if;
};

local interface implicit
{
    as in(break internal, delegate is, explicit lock, case* namespace, decimal* new, delegate* null,
                explicit* operator, int override, int params, int private)
        throws fixed, foreach;
};

const int protected = 0;
const int public = 0;

};

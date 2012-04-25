// **********************************************************************
//
// Copyright (c) 2003-2012 ZeroC, Inc. All rights reserved.
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
    int if;
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

exception BaseMethods
{
    int Data;
    int HelpLink;
    int InnerException;
    int Message;
    int Source;
    int StackTrace;
    int TargetSite;
    int HResult;
    int Equals;
    int GetBaseException;
    int GetHashCode;
    int GetObjectData;
    int GetType;
    int ReferenceEquals;
    int ToString;
};

local interface implicit
{
    as in(break internal, delegate is, explicit lock, case* namespace, decimal* new, delegate* null,
                explicit* operator, int override, int params, int private)
        throws fixed, foreach;
};

const int protected = 0;
const int public = 0;

//
// System as inner module.
//
module System
{

interface Test
{
    void op();
};

};

};

//
// System as outer module.
//
module System
{

interface Test
{
    void op();
};

};

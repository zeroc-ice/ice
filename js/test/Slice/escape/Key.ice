// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#pragma once

module await
{

enum var
{
    base
};

struct break
{
    int while;
};

interface case
{
    ["amd"] void catch(int checked, out int continue);
};

interface typeof
{
    void default();
};

class delete
{
    int if;
    case* else;
    int export;
};

interface explicit extends typeof, case
{
};

dictionary<string, break> while;

class package
{
    optional(1) break for;
    optional(2) var goto;
    optional(3) explicit if;
    optional(5) while internal;
    optional(7) string debugger;
    optional(8) explicit* null;
};

interface optionalParams
{
    optional(1) break for(optional(2) var goto,
                          optional(3) explicit if,
                          optional(5) while internal,
                          optional(7) string namespace,
                          optional(8) explicit* null);

    ["amd"]
    optional(1) break continue(optional(2) var goto,
                               optional(3) explicit if,
                               optional(5) while internal,
                               optional(7) string namespace,
                               optional(8) explicit* null);

    optional(1) break in(out optional(2) var goto,
                         out optional(3) explicit if,
                         out optional(5) while internal,
                         out optional(7) string namespace,
                         out optional(8) explicit* null);

    ["amd"]
    optional(1) break foreach(out optional(2) var goto,
                              out optional(3) explicit if,
                              out optional(5) while internal,
                              out optional(7) string namespace,
                              out optional(8) explicit* null);
};

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
    var in(break internal, delete is, explicit lock, case* namespace, typeof* new, delete null,
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

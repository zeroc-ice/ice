//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#pragma once

[["suppress-warning:deprecated"]]
module abstract
{

enum as
{
    base
}

struct break
{
    int readonly;
}

interface case
{
    ["amd"] void catch(int checked, out int continue);
}

interface decimal
{
    void default();
}

class delegate
{
    int if;
    case* else;
    int event;
}

interface explicit extends decimal, case
{
}

dictionary<string, break> while;

class optionalMembers
{
    optional(1) break for;
    optional(2) as goto;
    optional(3) explicit if;
    optional(5) while internal;
    optional(7) string namespace;
    optional(8) explicit* null;
}

interface optionalParams
{
    optional(1) break for(optional(2) as goto,
                          optional(3) explicit if,
                          optional(5) while internal,
                          optional(7) string namespace,
                          optional(8) explicit* null);

    ["amd"]
    optional(1) break continue(optional(2) as goto,
                               optional(3) explicit if,
                               optional(5) while internal,
                               optional(7) string namespace,
                               optional(8) explicit* null);

    optional(1) break in(out optional(2) as goto,
                         out optional(3) explicit if,
                         out optional(5) while internal,
                         out optional(7) string namespace,
                         out optional(8) explicit* null);

    ["amd"]
    optional(1) break foreach(out optional(2) as goto,
                              out optional(3) explicit if,
                              out optional(5) while internal,
                              out optional(7) string namespace,
                              out optional(8) explicit* null);
}

exception fixed
{
    int for;
}

exception foreach extends fixed
{
    int goto;
    int if;
}

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
}

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
}

}

}

//
// System as outer module.
//
module System
{

interface Test
{
    void op();
}

}

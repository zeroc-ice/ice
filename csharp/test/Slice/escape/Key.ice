//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#pragma once

[[preserve-case]]

module ZeroC::Slice::Test::Escape
{

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
    [amd] void catch(int checked, out int continue);
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

interface explicit : decimal, case
{
}

dictionary<string, break> while;

class taggedMembers
{
    tag(1) break? for;
    tag(2) as? goto;
    tag(3) explicit* if;
    tag(5) while? internal;
    tag(7) string? namespace;
    tag(8) explicit* null;
}

interface taggedParams
{
    tag(1) break? for(tag(2) as? goto,
                      tag(3) explicit* if,
                      tag(5) while? internal,
                      tag(7) string? namespace,
                      tag(8) explicit* null);

    [amd]
    tag(1) break? continue(tag(2) as? goto,
                        tag(3) explicit* if,
                        tag(5) while? internal,
                        tag(7) string? namespace,
                        tag(8) explicit* null);

    (tag(1) break? r1, tag(2) as? goto, tag(3) explicit* if,
                       tag(5) while? internal,
                       tag(7) string? namespace,
                       tag(8) explicit* null) in();

    [amd]
    (tag(1) break? r1, tag(2) as? goto, tag(3) explicit* if,
                       tag(5) while? internal,
                       tag(7) string? namespace,
                       tag(8) explicit* null) foreach();
}

exception fixed
{
    int for;
}

exception foreach : fixed
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
const int public = 1;

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

}

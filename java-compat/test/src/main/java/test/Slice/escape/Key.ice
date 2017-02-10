// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#pragma once

[["java:package:test.Slice.escape"]]
module abstract
{

enum assert
{
    boolean
};

struct break
{
    int case;
};

interface catch
{
    ["amd"] void checkedCast(int clone, out int continue);
};

interface default
{
    void do();
};

class else
{
    int if;
    default* equals;
    int final;
};

interface finalize extends default, catch
{
};
sequence<assert> for;
dictionary<string, assert> goto;

exception hashCode
{
    int if;
};

exception import extends hashCode
{
    int instanceof;
    int native;
};

local interface new
{
    assert notify( break notifyAll, else null, finalize package, else* private, finalize * protected, catch* public,
                default* return, int static, int strictfp, int super)
        throws hashCode, import;
};

const int switch = 0;
const int synchronized = 0;
const int this = 0;
const int throw = 0;
const int toString = 0;
const int try = 0;
const int uncheckedCast = 0;
const int volatile = 0;
const int wait = 0;
const int while = 0;
const int finally = 0;
const int getClass = 0;

};

// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

[["underscore"]]

module Clash
{

interface Intf
{
    void context();
    void current();
    void response();
    void upCast();
    void typeId();
    void del();
    void cookie();
    void sync();
    void inS();
    void istr();
    void ok();
    void target();

    void op(string context, string current, string response, string ex, string sent, string cookie,
            string sync, string result, string istr, string ostr, string target, optional(1) string proxy, optional(2) string ok);
    void opOut(out string context, out string current, out string response, out string ex,
               out string sent, out string cookie, out string sync, out string result, out string istr,
               out string ostr, out string target, out optional(1) string proxy, out optional(2) string ok);
}

class Cls
{
    Intf* s;
    string context;
    int current;
    short response;
    string upCast;
    int typeId;
    short del;
    optional(1) short cookie;
    string ex;
    int result;
    string istr;
    string ostr;
    string inS;
    string in;
    string proxy;
    optional(2) int stream;
    string target;
}

struct St
{
    string v;
    short istr;
    int ostr;
    int rhs;
}

exception Ex
{
    short istr;
    int ostr;
}

}

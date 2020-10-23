//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

[[suppress-warning(reserved-identifier)]]

[[preserve-case]]

module ZeroC::Slice::Test::Escape::Clash
{

interface Intf
{
    void context();
    void current();
    void response();
    void typeId();
    void del();
    void cookie();
    void sync();
    void inS();
    void istr();
    void obj();

    void op(string context, string current, string response, string ex, string sent, string cookie,
            string sync, string result, string istr, string ostr, tag(1) string? proxy, tag(2) int? obj);
    (string context, string current, string response,
            string ex, string sent, string cookie,
            string sync, string result, string istr,
            string ostr, tag(1) string? proxy, tag(2) int? obj) opReturn();
}

class Cls
{
    Intf* s;
    string context;
    int current;
    short response;
    int typeId;
    short del;
    tag(1) short? cookie;
    string ex;
    int result;
    string istr;
    string ostr;
    string inS;
    string in;
    string proxy;
    int obj;
    int getCookie;
    string clone;
}

struct St
{
    string v;
    short istr;
    int ostr;
    int rhs;
    string hashCode;
    int clone;
}

exception Ex
{
    short istr;
    int ostr;
    string cause;
}

}

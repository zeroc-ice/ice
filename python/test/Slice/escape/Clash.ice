// Copyright (c) ZeroC, Inc.

["python:identifier:generated.test.Slice.escape.Test"]
module Test
{
    class field;

    interface Intf
    {
        void context();
        void current();
        void response();
        void upCast();
        void typeId();
        ["python:identifier:_del"] void del();
        void cookie();
        void sync();
        void inS();
        void istr();

        void op(string context, string current, string response, string ex, string sent, string cookie,
            string sync, string result, string istr, string ostr, optional(1) string proxy);
        void opOut(out string context, out string current, out string response, out string ex,
            out string sent, out string cookie, out string sync, out string result, out string istr,
            out string ostr, out optional(1) string proxy);
        void opField(field name, field value);
    }

    class Cls
    {
        Intf* s;
        string context;
        int current;
        short response;
        string upCast;
        int typeId;
        ["python:identifier:_del"] short del;
        optional(1) short cookie;
        string ex;
        int result;
        string istr;
        string ostr;
        string inS;
        ["python:identifier:_in"] string in;
        string proxy;
    }

    struct St
    {
        string v;
        short istr;
        int ostr;
        int rhs;
        string other;
    }

    exception Ex
    {
        short istr;
        int ostr;
    }

    class field
    {
        St name;
        St value;
    }

    sequence<int> IntSeq;

    interface Sequence
    {
        void sendIntSeq(IntSeq seq);
        void abstractmethod();
    }

    interface Awaitable
    {
        void op();
    }
}

// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

interface i1 {
    void op();
    void op();
};

interface i2 {
    void op();
    void oP();
};

class c1 {
    long l;
    void l();
};

class c2 {
    long l;
    void L();
};

module m1 {};
module m1 {};
module M1 {};
module c1 {};
module C1 {};

interface i3 {
    void op(long aa, int aa);
    void op2(long bb, out int BB);
};

interface i4 {
    void I4();
};

interface i5 {
    void i5();
};

interface i6 {
    void op();
};

interface i7 extends i6 {
    void op();
};

interface i8 extends i6 {
    void OP();
};

class c3 {
    long l;
};

class c4 extends c3 {
    void l();
};

class c5 extends c3 {
    void L();
};

class c6 {
    long l;
    void L();
};

class c7 extends c3 {
    long l;
};

class c8 extends c3 {
    long L;
};

class c9 {
     void L();
     long l;
};

exception e1 {
    long l;
    string l;
};

exception e2 {
    long l;
    string L;
};

exception e3 {
    long e3;
};

exception e4 {
    long E4;
};

exception e5 {
    long l;
};

exception e6 extends e5 {
    string l;
};

exception e7 extends e5 {
    string L;
};

struct s1 {
    long l;
    string l;
};

struct s2 {
    long l;
    string L;
};

struct s3 {
    long s3;
    string x;
};

struct s4 {
    long S4;
    string x;
};

sequence<long> ls;
sequence<long> LS;
sequence<long> m1;
sequence<long> M1;

dictionary<long, string> d;
dictionary<long, string> D;
dictionary<long, string> m1;
dictionary<long, string> M1;

enum en1 { red };
enum eN1 { red };
enum m1 { green };
enum M1 { blue };

enum en2 { en1 };
enum en3 { EN1 };
enum en4 { m1 };
enum en5 { M1 };

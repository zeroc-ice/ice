// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************



module Test
{

module M1
{
};
module m1
{
};

struct m2
{
    int i;
};
module M2
{
};
class m2
{
};

interface i1
{
};
interface I1
{
};
interface i1;

exception e1
{
};
exception E1
{
};

interface i2;
struct I2
{
    int i;
};

interface i3;
sequence<int> I3;

interface i4;
dictionary<int, int> I4;

interface i5;
enum I5 { x };

interface i6;
enum e { I6 };

interface i7;
const long I7 = 1;

interface i8;
interface i9
{
    I8* op();
};

interface i10;
interface i11
{
    I10 op();
};

interface b1
{
    void op();
};
interface b2
{
    void OP();
};
interface D extends b1, b2
{
};

interface i12
{
    void op();
    void OP();
};

interface i13
{
    void I13();
};

class c1
{
    int i;
    int I;
};

class c2
{
   int C2;
};

class c3 extends c2
{
   int c2;
};

exception e2
{
    int i;
    int I;
};

exception e3
{
    int E3;
};

struct s1
{
    int i;
    int I;
};

struct s2
{
    int S2;
};

interface i14
{
    void op(int i, int I);
};

interface i15
{
    void op(int Op);
};

};

// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

module Test
{

const byte C1 = 0;
const short C2 = 0;
const int C3 = -1;
const long C4 = 0x80000001;
const float C5 = 1.1;
const long C6 = 2;

enum E { e1, e2, e3 = 4 };
enum Ebis { e2 };
enum Eter { e2 };

class C
{
    optional string m1;             // missing tag
    optional() int m2;              // missing tag
    optional(abc) bool m3;          // invalid tag
    optional(0x80000000) short m4;  // out of range
    optional(-0x80000001) long m5;  // out of range
    optional(-1) float m6;          // out of range
    optional(C1) string m7;         // ok
    optional(C2) string m8;         // duplicate tag
    optional(C3) double m9;         // invalid tag
    optional(C4) byte m10;          // out of range
    optional(C5) bool m11;          // invalid tag
    optional(C6) bool m12;          // ok
    optional(E::e1) int m13;        // duplicate tag
    optional(e2) int m14;           // ambiguous
    optional(e3) int m15;           // ok
    optional(4) int m16;            // duplicate tag
};

class Base
{
    optional(E::e2) int b1;
};

class Derived extends Base
{
    optional(1) int d1;
};

class Ex
{
    optional string m1;             // missing tag
    optional() int m2;              // missing tag
    optional(abc) bool m3;          // invalid tag
    optional(0x80000000) short m4;  // out of range
    optional(-0x80000001) long m5;  // out of range
    optional(-1) float m6;          // out of range
    optional(C1) string m7;         // ok
    optional(C2) string m8;         // duplicate tag
    optional(C3) double m9;         // invalid tag
    optional(C4) byte m10;          // out of range
    optional(C5) bool m11;          // invalid tag
    optional(C6) bool m12;          // ok
    optional(E::e1) int m13;        // duplicate tag
    optional(E::e2) int m14;        // ok
    optional(e3) int m15;           // ok
    optional(4) int m16;            // duplicate tag
};

class BaseEx
{
    optional(E::e2) int b1;
};

class DerivedEx extends BaseEx
{
    optional(1) int d1;
};

struct S
{
    optional(1) int m1;             // not allowed in struct
    optional(2) int m2 = 2;         // not allowed in struct
};

};

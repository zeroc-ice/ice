//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

module Test
{

const byte C1 = 0;
const short C2 = 0;
const int C3 = -1;
const long C4 = 0x80000001;
const float C5 = 1.1;
const long C6 = 2;

enum E { e1, e2, e3 = 4 }
enum Ebis { e2 }
enum Eter { e2 }

class C
{
    tag string m1;             // missing tag
    tag() int m2;              // missing tag
    tag(abc) bool m3;          // invalid tag
    tag(0x80000000) short m4;  // out of range
    tag(-0x80000001) long m5;  // out of range
    tag(-1) float m6;          // out of range
    tag(C1) string m7;         // ok
    tag(C2) string m8;         // duplicate tag
    tag(C3) double m9;         // invalid tag
    tag(C4) byte m10;          // out of range
    tag(C5) bool m11;          // invalid tag
    tag(C6) bool m12;          // ok
    tag(E::e1) int m13;        // duplicate tag
    tag(e2) int m14;           // ambiguous
    tag(e3) int m15;           // ok
    tag(4) int m16;            // duplicate tag
}

class Base
{
    tag(E::e2) int b1;
}

class Derived extends Base
{
    tag(1) int d1;
}

class Ex
{
    tag string m1;             // missing tag
    tag() int m2;              // missing tag
    tag(abc) bool m3;          // invalid tag
    tag(0x80000000) short m4;  // out of range
    tag(-0x80000001) long m5;  // out of range
    tag(-1) float m6;          // out of range
    tag(C1) string m7;         // ok
    tag(C2) string m8;         // duplicate tag
    tag(C3) double m9;         // invalid tag
    tag(C4) byte m10;          // out of range
    tag(C5) bool m11;          // invalid tag
    tag(C6) bool m12;          // ok
    tag(E::e1) int m13;        // duplicate tag
    tag(E::e2) int m14;        // ok
    tag(e3) int m15;           // ok
    tag(4) int m16;            // duplicate tag
    optional(123) int m17;     // deprecated
}

class BaseEx
{
    tag(E::e2) int b1;
}

class DerivedEx extends BaseEx
{
    tag(1) int d1;
}

struct S
{
    tag(1) int m1;             // not allowed in struct
    tag(2) int m2 = 2;         // not allowed in struct
}

}

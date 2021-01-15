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

enum E : uint { e1, e2, e3 = 4, e4 = 0x100000000 }
enum Ebis : short { e2, e3 = -1 }
enum Eter { e2 }

class C
{
    tag string? m1;             // missing tag
    tag() int? m2;              // missing tag
    tag(abc) bool? m3;          // invalid tag
    tag(0x80000000) short? m4;  // out of range
    tag(-0x80000001) long? m5;  // out of range
    tag(-1) float? m6;          // out of range
    tag(C1) string? m7;         // ok
    tag(C2) string? m8;         // duplicate tag
    tag(C3) double? m9;         // invalid tag
    tag(C4) byte? m10;          // out of range
    tag(C5) bool? m11;          // invalid tag
    tag(C6) bool? m12;          // ok
    tag(E::e1) int? m13;        // duplicate tag
    tag(e2) int? m14;           // ambiguous
    tag(E::e3) int? m15;        // ok
    tag(4) int? m16;            // duplicate tag
    optional(123) int? m17;     // deprecated
    tag(E::e4) int? m18;        // out of range
    tag(EBis::e3) int? m19;     // out of range
}

class Base
{
    tag(E::e2) int? b1;
}

class Derived : Base
{
    tag(1) int? d1;
}

class Ex
{
    tag string? m1;             // missing tag
    tag() int? m2;              // missing tag
    tag(abc) bool? m3;          // invalid tag
    tag(0x80000000) short? m4;  // out of range
    tag(-0x80000001) long? m5;  // out of range
    tag(-1) float? m6;          // out of range
    tag(C1) string? m7;         // ok
    tag(C2) string? m8;         // duplicate tag
    tag(C3) double? m9;         // invalid tag
    tag(C4) byte? m10;          // out of range
    tag(C5) bool? m11;          // invalid tag
    tag(C6) bool? m12;          // ok
    tag(E::e1) int? m13;        // duplicate tag
    tag(E::e2) int? m14;        // ok
    tag(E::e3) int? m15;        // ok
    tag(4) int? m16;            // duplicate tag
    optional(123) int? m17;     // deprecated
    tag(E::e4) int? m18;        // out of range
    tag(EBis::e3) int? m19;     // out of range
}

class BaseEx
{
    tag(E::e2) int? b1;
}

class DerivedEx : BaseEx
{
    tag(1) int? d1;
}

struct S
{
    tag(1) int? m1;             // not allowed in struct
    tag(2) int? m2 = 2;         // not allowed in struct
}

class OnlyDeclared;

sequence<byte> bs;
sequence<C> cs;

struct HasClassMembers
{
    int m1;
    string m2;
    C m3;
    Derived m4;
}

class TaggedClassMembers
{
    tag(1) int? m1;                 // ok
    tag(2) S? m2;                   // ok
    tag(3) C? m3;                   // tagged class
    tag(4) Derived? m4;             // tagged class
    tag(5) HasClassMembers? m5;     // tagging type that uses classes
    tag(6) TaggedClassMembers? m6;  // tagged class
    tag(7) AnyClass? m7;            // tagged class
    tag(8) OnlyDeclared? m8;        // tagged class

    tag(9) bs? m9;                  // ok
    tag(10) cs? m10;                // tagging type that uses classes
}

}

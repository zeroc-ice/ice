// Copyright (c) ZeroC, Inc.

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

    class C;

    struct ClassWrapper
    {
        C c;
    }

    interface I {}

    sequence<bool> BoolSeq;
    sequence<C> ClassSeq;
    sequence<ClassWrapper> ClassWrapperSeq;
    dictionary<int, bool> BoolDict;
    dictionary<int, C> ClassDict;

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
        optional(C3) double m9;         // out of range
        optional(C4) byte m10;          // out of range
        optional(C5) bool m11;          // invalid tag
        optional(C6) bool m12;          // ok
        optional(E::e1) int m13;        // duplicate tag
        optional(e2) int m14;           // ambiguous
        optional(e3) int m15;           // ok
        optional(4) int m16;            // duplicate tag
    }

    class Base
    {
        optional(E::e2) int b1;
    }

    class Derived extends Base
    {
        optional(1) int d1;
    }

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
        optional(C3) double m9;         // out of range
        optional(C4) byte m10;          // out of range
        optional(C5) bool m11;          // invalid tag
        optional(C6) bool m12;          // ok
        optional(E::e1) int m13;        // duplicate tag
        optional(E::e2) int m14;        // ok
        optional(e3) int m15;           // ok
        optional(4) int m16;            // duplicate tag
    }

    class BaseEx
    {
        optional(E::e2) int b1;
    }

    class DerivedEx extends BaseEx
    {
        optional(1) int d1;
    }

    struct S
    {
        optional(1) int m1;             // not allowed in struct
        optional(2) int m2 = 2;         // not allowed in struct
    }

    class D
    {
        optional(1) bool m1;            // ok
        optional(2) float m2;           // ok
        optional(3) string m3;          // ok
        optional(4) Object m4;          // cannot tag class types
        optional(5) Object* m5;         // ok
        optional(6) Value m6;           // cannot tag class types

        optional(7) C m7;               // cannot tag class types
        optional(8) I* m8;              // ok
        optional(9) S m9;               // ok
        optional(10) E m10;             // ok

        optional(11) BoolSeq m11;       // ok
        optional(12) ClassSeq m12;      // cannot tag class types
        optional(13) ClassWrapper m13;  // cannot tag class types
        optional(14) BoolDict m14;      // ok
        optional(15) ClassDict m15;     // cannot tag class types
    }
}

// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
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

enum E { e1, e2, e3 }
enum Ebis { e1 }

interface I
{
    optional string r1();             // missing tag
    optional() int r2();              // missing tag
    optional(abc) bool r3();          // invalid tag
    optional(0x80000000) short r4();  // out of range
    optional(-0x80000001) long r5();  // out of range
    optional(-1) float r6();          // out of range
    optional(C1) string r7();         // ok
    optional(C3) double r8();         // invalid tag
    optional(C4) byte r9();           // out of range
    optional(C5) bool r10();          // invalid tag
    optional(C6) bool r11();          // ok
    optional(E::e1) int r12();           // ok
    optional(E::e2) void r13();          // syntax error

    void i1(optional string p);             // missing tag
    void i2(optional() int p);              // missing tag
    void i3(optional(abc) bool p);          // invalid tag
    void i4(optional(0x80000000) short p);  // out of range
    void i5(optional(-0x80000001) long p);  // out of range
    void i6(optional(-1) float p);          // out of range
    void i7(optional(C1) string p);         // ok
    void i8(optional(C3) double p);         // invalid tag
    void i9(optional(C4) byte p);           // out of range
    void i10(optional(C5) bool p);          // invalid tag
    void i11(optional(C6) bool p);          // ok
    void i12(optional(e2) int p);           // ok (warning)

    void o1(out optional string p);             // missing tag
    void o2(out optional() int p);              // missing tag
    void o3(out optional(abc) bool p);          // invalid tag
    void o4(out optional(0x80000000) short p);  // out of range
    void o5(out optional(-0x80000001) long p);  // out of range
    void o6(out optional(-1) float p);          // out of range
    void o7(out optional(C1) string p);         // ok
    void o8(out optional(C3) double p);         // invalid tag
    void o9(out optional(C4) byte p);           // out of range
    void o10(out optional(C5) bool p);          // invalid tag
    void o11(out optional(C6) bool p);          // ok
    void o12(out optional(e1) int p);           // ambiguous

    optional(1) int io1(optional(2) int p, out optional(3) int o);      // ok
    optional(1) int io2(out optional(2) int p, out optional(3) int o);  // ok
    optional(1) int io3(optional(2) int p, out optional(1) int o);      // duplicate tag
    optional(1) int io4(out optional(2) int p, out optional(2) int o);  // duplicate tag
    optional(2) int io5(out optional(1) int p, out optional(2) int o);  // duplicate tag
    optional(C1) int io6(optional(E::e2) int p, out optional(E::e1) int o);   // duplicate tag
}

}

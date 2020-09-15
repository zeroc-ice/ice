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

enum E : uint { e1, e2, e3, e4 = 0x100000000 }
enum Ebis { e1 }

interface I
{
    tag string? r1();             // missing tag
    tag() int? r2();              // missing tag
    tag(abc) bool? r3();          // invalid tag
    tag(0x80000000) short? r4();  // out of range
    tag(-0x80000001) long? r5();  // out of range
    tag(-1) float? r6();          // out of range
    tag(C1) string? r7();         // ok
    tag(C3) double? r8();         // invalid tag
    tag(C4) byte? r9();           // out of range
    tag(C5) bool? r10();          // invalid tag
    tag(C6) bool? r11();          // ok
    tag(E::e1) int? r12();        // ok
    tag(E::e2) void r13();        // syntax error

    void i1(tag string? p);             // missing tag
    void i2(tag() int? p);              // missing tag
    void i3(tag(abc) bool? p);          // invalid tag
    void i4(tag(0x80000000) short? p);  // out of range
    void i5(tag(-0x80000001) long? p);  // out of range
    void i6(tag(-1) float? p);          // out of range
    void i7(tag(C1) string? p);         // ok
    void i8(tag(C3) double? p);         // invalid tag
    void i9(tag(C4) byte? p);           // out of range
    void i10(tag(C5) bool? p);          // invalid tag
    void i11(tag(C6) bool? p);          // ok
    void i12(tag(e2) int? p);           // ok (warning)

    void o1(out tag string?p);              // missing tag
    void o2(out tag() int? p);              // missing tag
    void o3(out tag(abc) bool? p);          // invalid tag
    void o4(out tag(0x80000000) short? p);  // out of range
    void o5(out tag(-0x80000001) long? p);  // out of range
    void o6(out tag(-1) float? p);          // out of range
    void o7(out tag(C1) string? p);         // ok
    void o8(out tag(C3) double? p);         // invalid tag
    void o9(out tag(C4) byte? p);           // out of range
    void o10(out tag(C5) bool? p);          // invalid tag
    void o11(out tag(C6) bool? p);          // ok
    void o12(out tag(e1) int? p);           // ambiguous
    void o13(out tag(E::e4) int? p);        // out of range

    tag(1) int? io1(tag(2) int? p, out tag(3) int? o);      // ok
    tag(1) int? io2(out tag(2) int? p, out tag(3) int? o);  // ok
    tag(1) int? io3(tag(2) int? p, out tag(1) int? o);      // duplicate tag
    tag(1) int? io4(out tag(2) int? p, out tag(2) int? o);  // duplicate tag
    tag(2) int? io5(out tag(1) int? p, out tag(2) int? o);  // duplicate tag
    tag(C1) int? io6(tag(E::e2) int? p, out tag(E::e1) int? o);   // duplicate tag

    optional(1) int w();            // deprecated
    int wi(optional(1) int p);      // deprecated
    int wo(out optional(1) int p);  // deprecated
}

class OnlyDeclared;

class C
{
    long l;
}

class Derived : C
{
    int i;
}

sequence<byte> bs;
sequence<C> cs;

struct S
{
    string s1;
    string s2;
}

struct HasClassMembers
{
    int m1;
    string m2;
    C m3;
    Derived m4;
}

interface TaggedClassMembers
{
    void op1(tag(1) int? p);                   // ok
    void op2(tag(2) S? p);                     // ok
    void op3(tag(3) C? p);                     // tagged class
    void op4(tag(4) Derived? p);               // tagged class
    void op5(tag(5) HasClassMembers? p);       // tagging type that uses classes
    void op7(tag(6) AnyClass? p);              // tagged class
    void op8(tag(7) OnlyDeclared? p);          // tagged class
    void op9(tag(8) bs? p);                    // ok
    void op10(tag(9) cs? p);                   // tagging type that uses classes

    tag(1) C? op11();                          // tagged class
    tag(2) AnyClass? op12();                   // tagged class
    tag(7) AnyClass? op13(tag(9) C? p);        // tagged classes

    (tag(1) C? r1, tag(2) AnyClass? r2) op14(tag(1) cs? p1, tag(2) bs? p2, tag(3) C? p3); // tagged classes
}

}

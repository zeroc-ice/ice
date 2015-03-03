module Test
{

enum E { E1, E2, E3 };

struct S1
{
    float m1;
    long m2;
    string m3;
    E m4;
};

sequence<bool> Seq1;
dictionary<byte, bool> D1;

interface I {};

class C1 implements I
{
    C1 c1m1;
    C1 c1m2;
    C1 c1m3;
    C1 c1m4;
    C1 c1m5;
    C1 c1m6;
    C1 c1m7;
    C1 c1m8;
    C1 c1m9;
    C1 c1m10;
    C1 c1m11;
    C1 c1m12;
    C1 c1m13;
    C1 c1m14;
    C1 c1m15;
    C1 c1m16;
};

class C2 extends C1
{
    C1 c2m1;
    C1 c2m2;
    C1 c2m3;
    C1 c2m4;
    C1 c2m5;
    C1 c2m6;
    C1 c2m7;
    C1 c2m8;
    C1 c2m9;
    C1 c2m10;
    C1 c2m11;
    C1 c2m12;
    C1 c2m13;
    C1 c2m14;
    C1 c2m15;
    C1 c2m16;
};

struct S2
{
    C1 m1;
    C1 m2;
    C1 m3;
    C1 m4;
    C1 m5;
    C1 m6;
    C1 m7;
    C1 m8;
    C1 m9;
    C1 m10;
    C1 m11;
    C1 m12;
    C1 m13;
    C1 m14;
    C1 m15;
    C1 m16;
};

};

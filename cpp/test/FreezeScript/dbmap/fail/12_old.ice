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
    D1 c1m1;
    D1 c1m2;
    D1 c1m3;
    D1 c1m4;
    D1 c1m5;
    D1 c1m6;
    D1 c1m7;
    D1 c1m8;
    D1 c1m9;
    D1 c1m10;
    D1 c1m11;
    D1 c1m12;
    D1 c1m13;
    D1 c1m14;
    D1 c1m15;
    D1 c1m16;
};

class C2 extends C1
{
    D1 c2m1;
    D1 c2m2;
    D1 c2m3;
    D1 c2m4;
    D1 c2m5;
    D1 c2m6;
    D1 c2m7;
    D1 c2m8;
    D1 c2m9;
    D1 c2m10;
    D1 c2m11;
    D1 c2m12;
    D1 c2m13;
    D1 c2m14;
    D1 c2m15;
    D1 c2m16;
};

struct S2
{
    D1 m1;
    D1 m2;
    D1 m3;
    D1 m4;
    D1 m5;
    D1 m6;
    D1 m7;
    D1 m8;
    D1 m9;
    D1 m10;
    D1 m11;
    D1 m12;
    D1 m13;
    D1 m14;
    D1 m15;
    D1 m16;
};

};

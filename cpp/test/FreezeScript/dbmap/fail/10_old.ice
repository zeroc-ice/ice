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

sequence<S1> Seq1;
dictionary<byte, S1> D1;

interface I {};

class C1 implements I
{
    S1 c1m1;
    S1 c1m2;
    S1 c1m3;
    S1 c1m4;
    S1 c1m5;
    S1 c1m6;
    S1 c1m7;
    S1 c1m8;
    S1 c1m9;
    S1 c1m10;
    S1 c1m11;
    S1 c1m12;
    S1 c1m13;
    S1 c1m14;
    S1 c1m15;
    S1 c1m16;
};

class C2 extends C1
{
    S1 c2m1;
    S1 c2m2;
    S1 c2m3;
    S1 c2m4;
    S1 c2m5;
    S1 c2m6;
    S1 c2m7;
    S1 c2m8;
    S1 c2m9;
    S1 c2m10;
    S1 c2m11;
    S1 c2m12;
    S1 c2m13;
    S1 c2m14;
    S1 c2m15;
    S1 c2m16;
};

struct S2
{
    S1 m1;
    S1 m2;
    S1 m3;
    S1 m4;
    S1 m5;
    S1 m6;
    S1 m7;
    S1 m8;
    S1 m9;
    S1 m10;
    S1 m11;
    S1 m12;
    S1 m13;
    S1 m14;
    S1 m15;
    S1 m16;
};

};

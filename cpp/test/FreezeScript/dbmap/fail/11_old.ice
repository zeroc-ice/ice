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

sequence<string> Seq1;
dictionary<byte, Seq1> D1;

interface I {};

class C1 implements I
{
    Seq1 c1m1;
    Seq1 c1m2;
    Seq1 c1m3;
    Seq1 c1m4;
    Seq1 c1m5;
    Seq1 c1m6;
    Seq1 c1m7;
    Seq1 c1m8;
    Seq1 c1m9;
    Seq1 c1m10;
    Seq1 c1m11;
    Seq1 c1m12;
    Seq1 c1m13;
    Seq1 c1m14;
    Seq1 c1m15;
    Seq1 c1m16;
};

class C2 extends C1
{
    Seq1 c2m1;
    Seq1 c2m2;
    Seq1 c2m3;
    Seq1 c2m4;
    Seq1 c2m5;
    Seq1 c2m6;
    Seq1 c2m7;
    Seq1 c2m8;
    Seq1 c2m9;
    Seq1 c2m10;
    Seq1 c2m11;
    Seq1 c2m12;
    Seq1 c2m13;
    Seq1 c2m14;
    Seq1 c2m15;
    Seq1 c2m16;
};

struct S2
{
    Seq1 m1;
    Seq1 m2;
    Seq1 m3;
    Seq1 m4;
    Seq1 m5;
    Seq1 m6;
    Seq1 m7;
    Seq1 m8;
    Seq1 m9;
    Seq1 m10;
    Seq1 m11;
    Seq1 m12;
    Seq1 m13;
    Seq1 m14;
    Seq1 m15;
    Seq1 m16;
};

};

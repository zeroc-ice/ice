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
    bool c1m1;
    bool c1m2;
    bool c1m3;
    bool c1m4;
    bool c1m5;
    bool c1m6;
    bool c1m7;
    bool c1m8;
    bool c1m9;
    bool c1m10;
    bool c1m11;
    bool c1m12;
    bool c1m13;
    bool c1m14;
    bool c1m15;
    bool c1m16;
};

class C2 extends C1
{
    bool c2m1;
    bool c2m2;
    bool c2m3;
    bool c2m4;
    bool c2m5;
    bool c2m6;
    bool c2m7;
    bool c2m8;
    bool c2m9;
    bool c2m10;
    bool c2m11;
    bool c2m12;
    bool c2m13;
    bool c2m14;
    bool c2m15;
    bool c2m16;
};

struct S2
{
    bool m1;
    bool m2;
    bool m3;
    bool m4;
    bool m5;
    bool m6;
    bool m7;
    bool m8;
    bool m9;
    bool m10;
    bool m11;
    bool m12;
    bool m13;
    bool m14;
    bool m15;
    bool m16;
};

};

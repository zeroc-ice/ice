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

sequence<E> Seq1;
dictionary<byte, E> D1;

interface I {};

class C1 implements I
{
    E c1m1;
    E c1m2;
    E c1m3;
    E c1m4;
    E c1m5;
    E c1m6;
    E c1m7;
    E c1m8;
    E c1m9;
    E c1m10;
    E c1m11;
    E c1m12;
    E c1m13;
    E c1m14;
    E c1m15;
    E c1m16;
};

class C2 extends C1
{
    E c2m1;
    E c2m2;
    E c2m3;
    E c2m4;
    E c2m5;
    E c2m6;
    E c2m7;
    E c2m8;
    E c2m9;
    E c2m10;
    E c2m11;
    E c2m12;
    E c2m13;
    E c2m14;
    E c2m15;
    E c2m16;
};

struct S2
{
    E m1;
    E m2;
    E m3;
    E m4;
    E m5;
    E m6;
    E m7;
    E m8;
    E m9;
    E m10;
    E m11;
    E m12;
    E m13;
    E m14;
    E m15;
    E m16;
};

};

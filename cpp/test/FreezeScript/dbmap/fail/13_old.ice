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

sequence<Object*> Seq1;
dictionary<byte, Object*> D1;

interface I {};

class C1 implements I
{
    Object* c1m1;
    Object* c1m2;
    Object* c1m3;
    Object* c1m4;
    Object* c1m5;
    Object* c1m6;
    Object* c1m7;
    Object* c1m8;
    Object* c1m9;
    Object* c1m10;
    Object* c1m11;
    Object* c1m12;
    Object* c1m13;
    Object* c1m14;
    Object* c1m15;
    Object* c1m16;
};

class C2 extends C1
{
    Object* c2m1;
    Object* c2m2;
    Object* c2m3;
    Object* c2m4;
    Object* c2m5;
    Object* c2m6;
    Object* c2m7;
    Object* c2m8;
    Object* c2m9;
    Object* c2m10;
    Object* c2m11;
    Object* c2m12;
    Object* c2m13;
    Object* c2m14;
    Object* c2m15;
    Object* c2m16;
};

struct S2
{
    Object* m1;
    Object* m2;
    Object* m3;
    Object* m4;
    Object* m5;
    Object* m6;
    Object* m7;
    Object* m8;
    Object* m9;
    Object* m10;
    Object* m11;
    Object* m12;
    Object* m13;
    Object* m14;
    Object* m15;
    Object* m16;
};

};

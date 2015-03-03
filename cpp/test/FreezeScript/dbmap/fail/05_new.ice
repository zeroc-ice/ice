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

sequence<E> Seq1; // FAIL
dictionary<short, E> D1; // FAIL

interface I {};

class C1 implements I
{
    bool c1m1; // FAIL
    byte c1m2;
    short c1m3;
    int c1m4;
    long c1m5;
    float c1m6; // FAIL
    double c1m7; // FAIL
    string c1m8;
    E c1m9; // FAIL
    S1 c1m10; // FAIL
    Seq1 c1m11; // FAIL
    D1 c1m12; // FAIL
    Object* c1m13; // FAIL
    Object c1m14; // FAIL
    C1* c1m15; // FAIL
    C1 c1m16; // FAIL
};

class C2 extends C1
{
    bool c2m1; // FAIL
    byte c2m2;
    short c2m3;
    int c2m4;
    long c2m5;
    float c2m6; // FAIL
    double c2m7; // FAIL
    string c2m8;
    E c2m9; // FAIL
    S1 c2m10; // FAIL
    Seq1 c2m11; // FAIL
    D1 c2m12; // FAIL
    Object* c2m13; // FAIL
    Object c2m14; // FAIL
    C1* c2m15; // FAIL
    C1 c2m16; // FAIL
};

struct S2
{
    bool m1; // FAIL
    byte m2;
    short m3;
    int m4;
    long m5;
    float m6; // FAIL
    double m7; // FAIL
    string m8;
    E m9; // FAIL
    S1 m10; // FAIL
    Seq1 m11; // FAIL
    D1 m12; // FAIL
    Object* m13; // FAIL
    Object m14; // FAIL
    C1* m15; // FAIL
    C1 m16; // FAIL
};

};

module Test
{

sequence<byte> Seq1;
sequence<short> Seq2;
sequence<int> Seq3;
sequence<long> Seq4;

enum E1 { one, two, three };

dictionary<string, E1> D1;

struct S1
{
    bool b;
    int i;
};

class C1
{
    string s;
    float f;
    byte b;
};

class C2 extends C1
{
    int i;
    long l;
    double d;
};

};

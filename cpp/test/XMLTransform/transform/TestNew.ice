module Test
{

sequence<short> Seq1; // changed from byte to short
sequence<int> Seq2; // changed from short to int
sequence<long> Seq3; // changed from int to long
sequence<byte> Seq4; // changed from long to byte

enum E1 { one, two, four }; // three removed, four added

dictionary<string, E1> D1;

struct S1
{
    // b removed
    byte i; // changed from int to byte
    short s; // added
};

class C1
{
    string s; // unchanged
    // f removed
    short b; // changed from byte to short
};

class C2 extends C1
{
    int l; // changed from long to int
    int i; // moved
    // d removed
};

};

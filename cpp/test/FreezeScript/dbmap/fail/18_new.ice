module Test
{

class C1 {};
class C2 extends C1 {};
class C3 {};

struct S
{
    C3 m1;
    C2 m2; // FAIL
    C1 m3;
    Object m4;
};

};

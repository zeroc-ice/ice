sequence<long> ls;

struct s00
{
    ls ls;	// Changed meaning
};

struct s0
{
    ::ls ls;	// OK
};

struct s1
{
    ls mem;
    long ls;	// Changed meaning
};

struct s2
{
    ::ls mem;
    long ls;	// OK
};

module M
{

sequence<long> ls;

};

interface i1
{
    M::ls op();
    void M();	// Changed meaning
};

interface i2
{
    M::ls op();
    long M();	// Changed meaning
};

module N
{

    interface n1 extends i1 {};
    interface i1 {};		// Changed meaning
    interface i2 extends i2 {};	// Changed meaning

};

module O
{

    interface n1 extends ::i1 {};
    interface i1 {};			// OK
    interface i2 extends ::i2 {};	// OK

};

exception e1 {};

exception e2 {};

module E
{

    exception ee1 extends e1 {};
    exception e1 {};		// Changed meaning
    exception e2 extends e2 {};	// Changed meaning

};

interface c1 {};

class c2 {};

module C
{

    class cc1 implements c1 {};
    class c1 {};		// Changed meaning
    class c2 extends c2 {};	// Changed meaning

};

enum color { blue };

module B
{

    const color fc = blue;

    interface blue {};		// Changed meaning

};

enum counter { one, two };
sequence<counter> CounterSeq;

module SS
{
    sequence<CounterSeq> y;
    enum CounterSeq { a, b };
};

interface ParamTest
{
    void op(long param);
    void op2(counter param);
    void param(counter counter);	// Changed meaning
    void op3(long counter, counter x);	// Second "counter" is not a type
    void op4(long param, long param);
};

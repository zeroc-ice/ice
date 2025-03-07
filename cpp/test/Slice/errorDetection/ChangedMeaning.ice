// Copyright (c) ZeroC, Inc.
    
module Test
{
    sequence<long> ls;
<<<<<<< Updated upstream

=======
        
>>>>>>> Stashed changes
    struct s00
    {
        ls ls;      // OK as of Ice 3.6 (data member has its own scope)
        ls l;
    }
<<<<<<< Updated upstream

=======
        
>>>>>>> Stashed changes
    struct s0
    {
        Test::ls ls;        // OK
    }
<<<<<<< Updated upstream

=======
        
>>>>>>> Stashed changes
    struct s1
    {
        ls mem;
        long ls;    // OK as of Ice 3.6 (data member has its own scope)
    }
<<<<<<< Updated upstream

=======
        
>>>>>>> Stashed changes
    struct s2
    {
        Test::ls mem;
        long ls;    // OK
    }
<<<<<<< Updated upstream

=======
        
>>>>>>> Stashed changes
    module M
    {
        sequence<long> ls;
    }
<<<<<<< Updated upstream

=======
        
>>>>>>> Stashed changes
    interface i1
    {
        M::ls op();
        void M();   // Changed meaning
    }
<<<<<<< Updated upstream

=======
        
>>>>>>> Stashed changes
    interface i2
    {
        M::ls op();
        long M();   // Changed meaning
    }
<<<<<<< Updated upstream

=======
        
>>>>>>> Stashed changes
    module N
    {
        interface n1 extends i1 {}
        interface i1 {}            // Changed meaning
        interface i2 extends i2 {} // Changed meaning
    }
<<<<<<< Updated upstream

=======
        
>>>>>>> Stashed changes
    module O
    {
        interface n1 extends ::Test::i1 {}
        interface i1 {}                    // OK
        interface i2 extends ::Test::i2 {} // OK
    }
<<<<<<< Updated upstream
    exception e1 {}
    exception e2 {}
=======
        
    exception e1 {}
        
    exception e2 {}
        
>>>>>>> Stashed changes
    module E
    {
        exception ee1 extends e1 {}
        exception e1 {}            // Changed meaning
        exception e2 extends e2 {} // Changed meaning
    }
<<<<<<< Updated upstream
    interface c1 {}
    class c2 {}
=======
        
    interface c1 {}
        
    class c2 {}
        
>>>>>>> Stashed changes
    module C
    {
        class c1 {}                // Changed meaning
        class c2 extends c2 {}     // Changed meaning
    }
<<<<<<< Updated upstream
    enum color { blue }
=======
        
    enum color { blue }
        
>>>>>>> Stashed changes
    module B
    {
        const color fc = blue;
        interface blue {}          // OK as of Ice 3.7 (enumerators are in their enum's namespace)
    }
<<<<<<< Updated upstream
    enum counter { one, two }
    sequence<counter> CounterSeq;

=======
        
    enum counter { one, two }
    sequence<counter> CounterSeq;
        
>>>>>>> Stashed changes
    module SS
    {
        sequence<CounterSeq> y;
        enum CounterSeq { a, b }
    }
<<<<<<< Updated upstream

=======
        
>>>>>>> Stashed changes
    interface ParamTest
    {
        void op(long param);
        void op2(counter param);
        void param(counter counter);        // OK as of Ice 3.6 (parameters have their own scope)
        void op3(long counter, counter x);  // OK as of Ice 3.6.1 (second "counter" is not a type)
        void op4(long param, long param);
    }
<<<<<<< Updated upstream

=======
        
>>>>>>> Stashed changes
    sequence<int> IS;
    struct x
    {
        IS is;                              // OK as of Ice 3.6 (data member has its own scope)
    }
<<<<<<< Updated upstream

=======
        
>>>>>>> Stashed changes
    struct y
    {
        ::Test::IS is;                      // OK, nothing introduced
    }
<<<<<<< Updated upstream

=======
        
>>>>>>> Stashed changes
    interface Blah
    {
        void op1() throws ::Test::E::ee1;   // Nothing introduced
        void E();                           // OK
        void op2() throws E;                // Changed meaning
    }
<<<<<<< Updated upstream

=======
        
>>>>>>> Stashed changes
    interface Blah2
    {
        void op3() throws ::Test::E::ee1;   // Nothing introduced
        void E();                           // OK
        void op4() throws E::ee1;           // Changed meaning
    }
<<<<<<< Updated upstream

=======
        
>>>>>>> Stashed changes
    interface Blah3
    {
        void op5() throws E::ee1;           // Introduces E
        void E();                           // Changed meaning
    }
<<<<<<< Updated upstream

    module M1
    {
        enum smnpEnum { a }
=======
        
    module M1
    {
        enum smnpEnum { a }
            
>>>>>>> Stashed changes
        struct smnpStruct
        {
            smnpEnum e;
        }
<<<<<<< Updated upstream

        exception smnpException
        {
        }

=======
            
        exception smnpException
        {
        }
            
>>>>>>> Stashed changes
        module M2
        {
            enum C { C1, C2, C3 }
        }
    }
<<<<<<< Updated upstream

=======
        
>>>>>>> Stashed changes
    const Test::M1::M2::C MyConstant1 = Test::M1::M2::C::C2; // OK
    const ::Test::M1::M2::C MyConstant2 = Test::M1::M2::C::C2; // OK
    const Test::M1::M2::C MyConstant3 = ::Test::M1::M2::C::C2; // OK
    const ::Test::M1::M2::C MyConstant4 = ::Test::M1::M2::C::C2; // OK
<<<<<<< Updated upstream

=======
        
>>>>>>> Stashed changes
    interface smnpTest1Class
    {
        M1::smnpStruct smnpTest1Op1() throws M1::smnpException; // OK
    }
<<<<<<< Updated upstream

=======
        
>>>>>>> Stashed changes
    class Foo
    {
        string x;
        string X;
    }
<<<<<<< Updated upstream

=======
        
>>>>>>> Stashed changes
    struct Foo1
    {
        string x;
        string X;
    }
<<<<<<< Updated upstream

=======
        
>>>>>>> Stashed changes
    interface Foo2
    {
        void op1(int a, int A); // Changed meaning
        void op2(int Foo2); // OK
        void op3(int op3); // Ok
    }
<<<<<<< Updated upstream

=======
        
>>>>>>> Stashed changes
    exception Foo5
    {
        string x;
        int X; // Changed meaning
    }
}

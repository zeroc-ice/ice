// Copyright (c) ZeroC, Inc.
    
module Test
{
    interface i1
    {
        void op();
        void op();
    }
<<<<<<< Updated upstream

=======
        
>>>>>>> Stashed changes
    interface i2
    {
        void op();
        void oP();
    }
<<<<<<< Updated upstream

=======
        
>>>>>>> Stashed changes
    interface iice
    {
        void ice_isa();
    }
<<<<<<< Updated upstream

=======
        
>>>>>>> Stashed changes
    module m1
    {}
    module m1
    {}
    module M1
    {}
    module c1
    {}
    module C1
    {}
<<<<<<< Updated upstream
=======
        
>>>>>>> Stashed changes
    interface i3
    {
        void op(long aa, int aa);
        void op2(long bb, out int BB);
    }
<<<<<<< Updated upstream

=======
        
>>>>>>> Stashed changes
    interface i4
    {
        void I4();
    }
<<<<<<< Updated upstream

=======
        
>>>>>>> Stashed changes
    interface i5
    {
        void i5();
    }
<<<<<<< Updated upstream

=======
        
>>>>>>> Stashed changes
    interface i6
    {
        void op();
    }
<<<<<<< Updated upstream

=======
        
>>>>>>> Stashed changes
    interface i7 extends i6
    {
        void op();
    }
<<<<<<< Updated upstream

=======
        
>>>>>>> Stashed changes
    interface i8 extends i6
    {
        void OP();
    }
<<<<<<< Updated upstream

    class c3
    {
        long l;
    }

    class c7 extends c3
    {
        long l;
    }

    class c8 extends c3
    {
        long L;
    }

    exception e1
    {
        long l;
        string l;
    }

    exception e2
    {
        long l;
        string L;
    }

    exception e3
    {
        long e3;
    }

    exception e4
    {
        long E4;
    }

    exception e5
    {
        long l;
    }

    exception e6 extends e5
    {
        string l;
    }

=======
        
    class c3
    {
        long l;
    }
        
    class c7 extends c3
    {
        long l;
    }
        
    class c8 extends c3
    {
        long L;
    }
        
    exception e1
    {
        long l;
        string l;
    }
        
    exception e2
    {
        long l;
        string L;
    }
        
    exception e3
    {
        long e3;
    }
        
    exception e4
    {
        long E4;
    }
        
    exception e5
    {
        long l;
    }
        
    exception e6 extends e5
    {
        string l;
    }
        
>>>>>>> Stashed changes
    exception e7 extends e5
    {
        string L;
    }
<<<<<<< Updated upstream

=======
        
>>>>>>> Stashed changes
    struct s1
    {
        long l;
        string l;
    }
<<<<<<< Updated upstream

=======
        
>>>>>>> Stashed changes
    struct s2
    {
        long l;
        string L;
    }
<<<<<<< Updated upstream

=======
        
>>>>>>> Stashed changes
    struct s3
    {
        long s3;
        string x;
    }
<<<<<<< Updated upstream

=======
        
>>>>>>> Stashed changes
    struct s4
    {
        long S4;
        string x;
    }
<<<<<<< Updated upstream

=======
        
>>>>>>> Stashed changes
    sequence<long> ls;
    sequence<long> LS;
    sequence<long> m1;
    sequence<long> M1;
<<<<<<< Updated upstream

=======
        
>>>>>>> Stashed changes
    dictionary<long, string> d;
    dictionary<long, string> D;
    dictionary<long, string> m1;
    dictionary<long, string> M1;
<<<<<<< Updated upstream
=======
        
>>>>>>> Stashed changes
    enum en1 { red }
    enum eN1 { lilac }
    enum m1 { green }
    enum M1 { blue }
    enum en2 { yellow, en1, EN1}
<<<<<<< Updated upstream
=======
        
>>>>>>> Stashed changes
    module xxx::xx
    {
        interface Base
        {
            void op();
        }
<<<<<<< Updated upstream

        interface Derived extends Base
        {
        }

        interface Derived extends base
        {
        }

        exception e1
        {
        }

        exception e2 extends E1
        {
        }

=======
            
        interface Derived extends Base
        {
        }
            
        interface Derived extends base
        {
        }
            
        exception e1
        {
        }
            
        exception e2 extends E1
        {
        }
            
>>>>>>> Stashed changes
        sequence<long> s1;
        struct s
        {
            S1 x;
            xxx::xx::S1 y;
            xxx::XX::s1 z;
            xxx::XX::s1 w;
        }
<<<<<<< Updated upstream

=======
            
>>>>>>> Stashed changes
        struct s2
        {
            Derived* blah;
            derived* bletch;
        }
    }
<<<<<<< Updated upstream

=======
        
>>>>>>> Stashed changes
    interface Foo
    {
        void op(long param, string Param);
        void op2() throws e1;
        void op3() throws E1;
        void op4() throws Test::xxx::xx::e1;
        void op5() throws Test::xxx::xx::E1;
        void op6() throws Test::xxx::XX::e1;
        void op7() throws Test::XXX::xx::e1;
        void op8() throws ::Test::xxx::xx::e1;
        void op9() throws ::Test::xxx::xx::E1;
        void op10() throws ::Test::xxx::XX::e1;
        void op11() throws ::Test::XXX::xx::e1;
        void op12(long op12);
        void op13(long OP13);
    }
<<<<<<< Updated upstream

=======
        
>>>>>>> Stashed changes
    module CI
    {
        interface base1
        {
            void op();
        }
<<<<<<< Updated upstream

=======
            
>>>>>>> Stashed changes
        interface base2
        {
            void OP();
        }
<<<<<<< Updated upstream

=======
            
>>>>>>> Stashed changes
        interface derived extends base1, base2
        {
        }
    }
<<<<<<< Updated upstream

=======
        
>>>>>>> Stashed changes
    module DI
    {
        interface base
        {
            void aa(int AA);
            void xx(int base);
        }
<<<<<<< Updated upstream

=======
            
>>>>>>> Stashed changes
        class base1
        {
            int base1;
        }
<<<<<<< Updated upstream

=======
            
>>>>>>> Stashed changes
        struct Foo
        {
            int foo;
        }
<<<<<<< Updated upstream

=======
            
>>>>>>> Stashed changes
        struct Foo1
        {
            int Foo1;
        }
<<<<<<< Updated upstream

=======
            
>>>>>>> Stashed changes
        exception Bar
        {
            string bar;
        }
<<<<<<< Updated upstream

=======
            
>>>>>>> Stashed changes
        exception Bar1
        {
            string Bar1;
        }
<<<<<<< Updated upstream

=======
            
>>>>>>> Stashed changes
        class base2
        {
            int X;
        }
<<<<<<< Updated upstream

=======
            
>>>>>>> Stashed changes
        class derived2 extends base2
        {
            string x;
        }
    }
}

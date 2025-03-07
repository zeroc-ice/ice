// Copyright (c) ZeroC, Inc.
    
module Test
{
    interface Base1
    {
        void op();
        void op2();
        void ice_ping();
    }
<<<<<<< Updated upstream

=======
        
>>>>>>> Stashed changes
    interface Derived1 extends Base1
    {
        void op();                          // error
        long op();                          // error
        void foo();
    }
<<<<<<< Updated upstream

=======
        
>>>>>>> Stashed changes
    interface Base2
    {
        void op();
    }
<<<<<<< Updated upstream

=======
        
>>>>>>> Stashed changes
    interface D1 extends Base1
    {
        void foo();                         // OK
    }
<<<<<<< Updated upstream

=======
        
>>>>>>> Stashed changes
    interface D2 extends D1
    {
        void op();                          // error, op() in Base1
    }
<<<<<<< Updated upstream

=======
        
>>>>>>> Stashed changes
    interface D3 extends D1, Base2
    {
        void bar();                         // error, op() in Base1 and Base2
    }
<<<<<<< Updated upstream
=======
        
>>>>>>> Stashed changes
    class c1 { long l; }
    class c2 extends c1 { double l; }      // error
    class c3 extends c1 { double d; }      // OK
    class c4 extends c3 { short l; }       // error, l in c1
}

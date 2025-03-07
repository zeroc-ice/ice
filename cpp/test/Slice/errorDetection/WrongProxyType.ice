// Copyright (c) ZeroC, Inc.

module Test
{
    sequence<int> Seq;
    dictionary<bool, int> Dict;
<<<<<<< Updated upstream

=======

>>>>>>> Stashed changes
    interface I
    {
        Seq* f1();
        void f2(Seq*);
        void f3(out Seq*);
<<<<<<< Updated upstream

=======

>>>>>>> Stashed changes
        Dict* f4();
        void f5(Dict*);
        void f6(out Dict*);
        void op();
    }
}

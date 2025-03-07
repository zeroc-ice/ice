// Copyright (c) ZeroC, Inc.

#pragma once

module LocalTest
{
    class C1
    {
        int i;
    }
<<<<<<< Updated upstream

=======

>>>>>>> Stashed changes
    struct S1
    {
        C1 c1;
    }
<<<<<<< Updated upstream

=======

>>>>>>> Stashed changes
    sequence<C1> C1Seq;
    sequence<S1> S1Seq;
    dictionary<int, C1> C1Dict;
    dictionary<int, S1> S1Dict;
<<<<<<< Updated upstream

    sequence<C1Seq> C1SeqSeq;
    sequence<S1Seq> S1SeqSeq;

=======

    sequence<C1Seq> C1SeqSeq;
    sequence<S1Seq> S1SeqSeq;

>>>>>>> Stashed changes
    struct S2
    {
        S1 s1;
    }
<<<<<<< Updated upstream

=======

>>>>>>> Stashed changes
    struct S3
    {
        C1Seq c1seq;
    }
<<<<<<< Updated upstream

=======

>>>>>>> Stashed changes
    struct S4
    {
        S1Seq s1seq;
    }
<<<<<<< Updated upstream

=======

>>>>>>> Stashed changes
    struct S5
    {
        C1Dict c1dict;
    }
<<<<<<< Updated upstream

=======

>>>>>>> Stashed changes
    struct S6
    {
        S1Dict s1dict;
    }
<<<<<<< Updated upstream

=======

>>>>>>> Stashed changes
    struct S7
    {
        C1SeqSeq c1seqseq;
    }
<<<<<<< Updated upstream

=======

>>>>>>> Stashed changes
    struct S8
    {
        S1SeqSeq s1seqseq;
    }
<<<<<<< Updated upstream

=======

>>>>>>> Stashed changes
    class CB1
    {
        S1 s1;
    }
<<<<<<< Updated upstream

=======

>>>>>>> Stashed changes
    class CB2
    {
        C1Seq c1seq;
    }
<<<<<<< Updated upstream

=======

>>>>>>> Stashed changes
    class CB3
    {
        S1Seq s1seq;
    }
<<<<<<< Updated upstream

=======

>>>>>>> Stashed changes
    class CB4
    {
        C1Dict c1dict;
    }
<<<<<<< Updated upstream

=======

>>>>>>> Stashed changes
    class CB5
    {
        S1Dict s1dict;
    }
<<<<<<< Updated upstream

=======

>>>>>>> Stashed changes
    class CB6
    {
        C1SeqSeq c1seqseq;
    }
<<<<<<< Updated upstream

=======

>>>>>>> Stashed changes
    class CB7
    {
        S1SeqSeq s1seqseq;
    }
<<<<<<< Updated upstream

=======

>>>>>>> Stashed changes
    class CB8
    {
        S1 s1;
        C1Seq c1seq;
        S1Dict s1dict;
    }
<<<<<<< Updated upstream

=======

>>>>>>> Stashed changes
    struct StructKey
    {
        int i;
        int j;
    }
<<<<<<< Updated upstream

    dictionary<StructKey, C1> StructDict1;
    dictionary<StructKey, S1> StructDict2;

=======

    dictionary<StructKey, C1> StructDict1;
    dictionary<StructKey, S1> StructDict2;

>>>>>>> Stashed changes
    dictionary<int, C1Dict> C1DictDict;
    dictionary<int, S1Dict> S1DictDict;
}

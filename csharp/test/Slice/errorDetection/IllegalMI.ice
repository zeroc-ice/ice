//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

module M1
{

    interface A1
    {
        void ia1();
    }

    interface A2 : A1
    {
        void ia2();
    }

    interface B2 : A1
    {
        void ib2();
    }

    interface A3 : A2, B2
    {       // OK
        void ia3();
    }

}

// ----------------------------------------------------------------------

module M2
{

    interface A1
    {
        void ia1();
    }

    interface A2 : A1
    {
        void ia2();
        void op();
    }

    interface B2 : A1
    {
        void ib2();
        void op();
    }

    interface A3 : A2, B2         // Error
    {
        void ia3();
    }

}

// ----------------------------------------------------------------------

module M3
{

    interface A1
    {
        void ia1();
    }

    interface A2 : A1
    {
        void ia2();
        void op();
    }

    interface B2 : A1
    {
        void ib2();
    }

    interface A3 : A2, B2
    {
        void ia3();
        void op();                      // Error
    }

}

// ----------------------------------------------------------------------

module M4
{

    interface A1
    {
        void ia1();
    }

    interface A2 : A1
    {
        void ia2();
    }

    interface B2 : A1
    {
        void ib2();
        void op();
    }

    interface A3 : A2, B2
    {
        void ia3();
        void op();                      // Error
    }

}

// ----------------------------------------------------------------------

module M5
{

    interface A1
    {
        void ia1();
        void op();
    }

    interface A2 : A1
    {
        void ia2();
    }

    interface B2 : A1
    {
        void ib2();
    }

    interface A3 : A2, B2
    {
        void ia3();
        void op();                      // Error
    }

}

// ----------------------------------------------------------------------

module M6
{

    interface A1
    {
        void ia1();
    }

    interface B1
    {
        void ib1();
    }

    interface C1
    {
        void ic1();
    }

    interface A2 : A1, B1, C1 {   // OK
        void ia2();
    }
}

// ----------------------------------------------------------------------

module M7
{

    interface A1
    {
        void ia1();
        void op();
    }

    interface B1
    {
        void ib1();
        void op();
    }

    interface C1
    {
        void ic1();
    }

    interface A2 : A1, B1, C1     // Error
    {
        void ia2();
    }
}

// ----------------------------------------------------------------------

module M8
{

    interface A1
    {
        void ia1();
        void op();
    }

    interface B1
    {
        void ib1();
    }

    interface C1
    {
        void ic1();
        void op();
    }

    interface A2 : A1, B1, C1     // Error
    {
        void ia2();
    }

}

// ----------------------------------------------------------------------

module M9
{
    interface A1
    {
        void ia1();
    }

    interface B1
    {
        void ib1();
        void op();
    }

    interface C1
    {
        void ic1();
        void op();
    }

    interface A2 : A1, B1, C1     // Error
    {
        void ia2();
    }

}

// ----------------------------------------------------------------------

module M10
{

    interface A1
    {
        void ia1();
        void op();
    }

    interface B1
    {
        void ib1();
        void op();
    }

    interface C1
    {
        void ic1();
        void op();
    }

    interface A2 : A1, B1, C1     // Error
    {
        void ia2();
    }

}

// ----------------------------------------------------------------------

module M11
{

    interface A1
    {
        void ia1();
    }

    interface B1
    {
        void ib1();
    }

    interface C1
    {
        void ic1();
    }

    interface D1
    {
        void id1();
    }

    interface A2 : A1, B1, C1, D1 {       // OK
        void ia2();
    }
}

// ----------------------------------------------------------------------

module M12
{

    interface A1
    {
        void ia1();
        void op();
    }

    interface B1
    {
        void ib1();
        void op();
    }

    interface C1
    {
        void ic1();
    }

    interface D1
    {
        void id1();
    }

    interface A2 : A1, B1, C1, D1         // Error
    {
        void ia2();
    }

}

// ----------------------------------------------------------------------

module M13
{

    interface A1
    {
        void ia1();
        void op();
    }

    interface B1
    {
        void ib1();
    }

    interface C1
    {
        void ic1();
        void op();
    }

    interface D1
    {
        void id1();
    }

    interface A2 : A1, B1, C1, D1         // Error
    {
        void ia2();
    }

}

// ----------------------------------------------------------------------

module M14
{
    interface A1
    {
        void ia1();
        void op();
    }

    interface B1
    {
        void ib1();
    }

    interface C1
    {
        void ic1();
    }

    interface D1
    {
        void id1();
        void op();
    }

    interface A2 : A1, B1, C1, D1         // Error
    {
        void ia2();
    }

}

// ----------------------------------------------------------------------

module M15
{

    interface A1
    {
        void ia1();
    }

    interface B1
    {
        void ib1();
        void op();
    }

    interface C1
    {
        void ic1();
        void op();
    }

    interface D1
    {
        void id1();
    }

    interface A2 : A1, B1, C1, D1         // Error
    {
        void ia2();
    }

}

// ----------------------------------------------------------------------

module M16
{

    interface A1
    {
        void ia1();
    }

    interface B1
    {
        void ib1();
        void op();
    }

    interface C1
    {
        void ic1();
    }

    interface D1
    {
        void id1();
        void op();
    }

    interface A2 : A1, B1, C1, D1         // Error
    {
        void ia2();
    }

}

// ----------------------------------------------------------------------

module M17
{

    interface A1
    {
        void ia1();
    }

    interface B1
    {
        void ib1();
    }

    interface C1
    {
        void ic1();
        void op();
    }

    interface D1
    {
        void id1();
        void op();
    }

    interface A2 : A1, B1, C1, D1         // Error
    {
        void ia2();
    }

}

// ----------------------------------------------------------------------

module M18
{

    interface A1
    {
        void ia1();
        void op();
    }

    interface B1
    {
        void ib1();
        void op();
    }

    interface C1
    {
        void ic1();
        void op();
    }

    interface D1
    {
        void id1();
    }

    interface A2 : A1, B1, C1, D1         // Error
    {
        void ia2();
    }

}

// ----------------------------------------------------------------------

module M19
{

    interface A1
    {
        void ia1();
        void op();
    }

    interface B1
    {
        void ib1();
        void op();
    }

    interface C1
    {
        void ic1();
    }

    interface D1
    {
        void id1();
        void op();
    }

    interface A2 : A1, B1, C1, D1
    {
        void ia2();
    }

}

// ----------------------------------------------------------------------

module M20
{

    interface A1
    {
        void ia1();
        void op();
    }

    interface B1
    {
        void ib1();
    }

    interface C1
    {
        void ic1();
        void op();
    }

    interface D1
    {
        void id1();
        void op();
    }

    interface A2 : A1, B1, C1, D1         // Error
    {
        void ia2();
    }

}

// ----------------------------------------------------------------------

module M21
{

    interface A1
    {
        void ia1();
    }

    interface B1
    {
        void ib1();
        void op();
    }

    interface C1
    {
        void ic1();
        void op();
    }

    interface D1
    {
        void id1();
        void op();
    }

    interface A2 : A1, B1, C1, D1         // Error
    {
        void ia2();
    }

}

// ----------------------------------------------------------------------

module M22
{

    interface A1
    {
        void ia1();
        void op();
    }

    interface B1
    {
        void ib1();
        void op();
    }

    interface C1
    {
        void ic1();
        void op();
    }

    interface D1
    {
        void id1();
        void op();
    }

    interface A2 : A1, B1, C1, D1         // Error
    {
        void ia2();
    }

}

// ----------------------------------------------------------------------

module M23
{

    interface A0
    {
        void ia0();
    }

    interface B0
    {
        void ib0();
    }

    interface A1 : A0, B0
    {
        void ia1();
    }

    interface B1
    {
        void ib1();
    }

    interface C1
    {
        void ic1();
    }

    interface D1
    {
        void id1();
    }

    interface A2 : A1, B1, C1, D1         // OK
    {
        void ia2();
    }

}

// ----------------------------------------------------------------------

module M24
{

    interface A0
    {
        void ia0();
        void op();
    }

    interface B0
    {
        void ib0();
    }

    interface A1 : A0, B0
    {
        void ia1();
    }

    interface B1
    {
        void ib1();
    }

    interface C1
    {
        void ic1();
    }

    interface D1
    {
        void id1();
    }

    interface A2 : A1, B1, C1, D1
    {
        void ia2();
        void op();                              // Error
    }

}

// ----------------------------------------------------------------------

module M25
{

    interface A0
    {
        void ia0();
    }

    interface B0
    {
        void ib0();
        void op();
    }

    interface A1 : A0, B0
    {
        void ia1();
    }

    interface B1
    {
        void ib1();
    }

    interface C1
    {
        void ic1();
    }

    interface D1
    {
        void id1();
    }

    interface A2 : A1, B1, C1, D1
    {
        void ia2();
        void op();                              // Error
    }

}

// ----------------------------------------------------------------------

module M26
{

                                        //       A0    B0  C0     D0    //
    interface A0 {                      //        \    /    \    /  \   //
        void ia0();                     //         \  /      \  /    \  //
    }                                  //          A1        B1     C1 //
                                        //         /  \      /  \    /  //
    interface B0 {                      //        /    \    /    \  /   //
        void ib0();                     //       A2    B2  C2     D2    //
    }                                  //        \    /    \    /      //
                                        //         \  /      \  /       //
    interface C0 {                      //          A3        B3        //
        void ic0();                     //           \        /         //
    }                                  //            \      /          //
                                        //             \    /           //
    interface D0 {                      //              \  /            //
        void id0();                     //               A4             //
    }

    interface A1 : A0, B0
    {
        void ia1();
    }

    interface B1 : C0, D0
    {
        void ib1();
    }

    interface C1 : D0
    {
        void ic1();
    }

    interface A2 : A1
    {
        void ia2();
    }

    interface B2 : A1
    {
        void ib2();
    }

    interface C2 : B1
    {
        void ic2();
    }

    interface D2 : B1, C1
    {
        void id2();
    }

    interface A3 : A2, B2
    {
        void ia3();
    }

    interface B3 : C2, D2
    {
        void ib3();
    }

    interface A4 : A3, B3         // OK
    {
        void ia4();
    }

}

// ----------------------------------------------------------------------

module M27
{

                                        //       A0    B0  C0     D0    //
    interface A0 {                      //        \    /    \    /  \   //
        void ia0();                     //         \  /      \  /    \  //
    }                                  //          A1        B1     C1 //
                                        //         /  \      /  \    /  //
    interface B0 {                      //        /    \    /    \  /   //
        void ib0();                     //       A2    B2  C2     D2    //
    }                                  //        \    /    \    /      //
                                        //         \  /      \  /       //
    interface C0 {                      //          A3        B3        //
        void ic0(); void op();          //           \        /         //
    }                                  //            \      /          //
                                        //             \    /           //
    interface D0 {                      //              \  /            //
        void id0();                     //               A4             //
    }

    interface A1 : A0, B0
    {
        void ia1();
    }

    interface B1 : C0, D0
    {
        void ib1();
    }

    interface C1 : D0
    {
        void ic1();
    }

    interface A2 : A1
    {
        void ia2(); void op();
    }

    interface B2 : A1
    {
        void ib2();
    }

    interface C2 : B1
    {
        void ic2();
    }

    interface D2 : B1, C1
    {
        void id2();
    }

    interface A3 : A2, B2
    {
        void ia3();
    }

    interface B3 : C2, D2
    {
        void ib3();
    }

    interface A4 : A3, B3         // Error
    {
        void ia4();
    }

}

// ----------------------------------------------------------------------

module M28
{

                                        //       A0    B0  C0     D0    //
    interface A0 {                      //        \    /    \    /  \   //
        void ia0();                     //         \  /      \  /    \  //
    }                                  //          A1        B1     C1 //
                                        //         /  \      /  \    /  //
    interface B0 {                      //        /    \    /    \  /   //
        void ib0(); void op();          //       A2    B2  C2     D2    //
    }                                  //        \    /    \    /      //
                                        //         \  /      \  /       //
    interface C0 {                      //          A3        B3        //
        void ic0(); void op();          //           \        /         //
    }                                  //            \      /          //
                                        //             \    /           //
    interface D0 {                      //              \  /            //
        void id0();                     //               A4             //
    }

    interface A1 : A0, B0
    {
        void ia1();
    }

    interface B1 : C0, D0
    {
        void ib1();
    }

    interface C1 : D0
    {
        void ic1();
    }

    interface A2 : A1
    {
        void ia2();
    }

    interface B2 : A1
    {
        void ib2();
    }

    interface C2 : B1
    {
        void ic2();
    }

    interface D2 : B1, C1
    {
        void id2();
    }

    interface A3 : A2, B2
    {
        void ia3();
    }

    interface B3 : C2, D2
    {
        void ib3();
    }

    interface A4 : A3, B3         // Error
    {
        void ia4();
    }

}

// ----------------------------------------------------------------------

module M29
{

                                        //       A0    B0  C0     D0    //
    interface A0 {                      //        \    /    \    /  \   //
        void ia0();                     //         \  /      \  /    \  //
    }                                  //          A1        B1     C1 //
                                        //         /  \      /  \    /  //
    interface B0 {                      //        /    \    /    \  /   //
        void ib0(); void op();          //       A2    B2  C2     D2    //
    }                                  //        \    /    \    /      //
                                        //         \  /      \  /       //
    interface C0 {                      //          A3        B3        //
        void ic0();                     //           \        /         //
    }                                  //            \      /          //
                                        //             \    /           //
    interface D0 {                      //              \  /            //
        void id0(); void op();          //               A4             //
    }

    interface A1 : A0, B0
    {
        void ia1();
    }

    interface B1 : C0, D0
    {
        void ib1();
    }

    interface C1 : D0
    {
        void ic1();
    }

    interface A2 : A1
    {
        void ia2();
    }

    interface B2 : A1
    {
        void ib2();
    }

    interface C2 : B1
    {
        void ic2();
    }

    interface D2 : B1, C1
    {
        void id2();
    }

    interface A3 : A2, B2
    {
        void ia3();
    }

    interface B3 : C2, D2
    {
        void ib3();
    }

    interface A4 : A3, B3         // Error
    {
        void ia4();
    }

}

module M30
{

                                        //       A0    B0  C0     D0    //
    interface A0 {                      //        \    /    \    /  \   //
        void ia0();                     //         \  /      \  /    \  //
    }                                  //          A1        B1     C1 //
                                        //         /  \      /  \    /  //
    interface B0 {                      //        /    \    /    \  /   //
        void ib0(); void op();          //       A2    B2  C2     D2    //
    }                                  //        \    /    \    /      //
                                        //         \  /      \  /       //
    interface C0 {                      //          A3        B3        //
        void ic0();                     //           \        /         //
    }                                  //            \      /          //
                                        //             \    /           //
    interface D0 {                      //              \  /            //
        void id0();                     //               A4             //
    }

    interface A1 : A0, B0
    {
        void ia1();
    }

    interface B1 : C0, D0
    {
        void ib1();
    }

    interface C1 : D0
    {
        void ic1(); void op();
    }

    interface A2 : A1
    {
        void ia2();
    }

    interface B2 : A1
    {
        void ib2();
    }

    interface C2 : B1
    {
        void ic2();
    }

    interface D2 : B1, C1
    {
        void id2();
    }

    interface A3 : A2, B2
    {
        void ia3();
    }

    interface B3 : C2, D2
    {
        void ib3();
    }

    interface A4 : A3, B3         // Error
    {
        void ia4();
    }

}

module M31
{

                                        //       A0    B0  C0     D0    //
    interface A0 {                      //        \    /    \    /  \   //
        void ia0();                     //         \  /      \  /    \  //
    }                                  //          A1        B1     C1 //
                                        //         /  \      /  \    /  //
    interface B0 {                      //        /    \    /    \  /   //
        void ib0();                     //       A2    B2  C2     D2    //
    }                                  //        \    /    \    /      //
                                        //         \  /      \  /       //
    interface C0 {                      //          A3        B3        //
        void ic0();                     //           \        /         //
    }                                  //            \      /          //
                                        //             \    /           //
    interface D0 {                      //              \  /            //
        void id0();                     //               A4             //
    }

    interface A1 : A0, B0
    {
        void ia1(); void op();
    }

    interface B1 : C0, D0
    {
        void ib1();
    }

    interface C1 : D0
    {
        void ic1(); void op();
    }

    interface A2 : A1
    {
        void ia2();
    }

    interface B2 : A1
    {
        void ib2();
    }

    interface C2 : B1
    {
        void ic2();
    }

    interface D2 : B1, C1
    {
        void id2();
    }

    interface A3 : A2, B2
    {
        void ia3();
    }

    interface B3 : C2, D2
    {
        void ib3();
    }

    interface A4 : A3, B3         // Error
    {
        void ia4();
    }
}

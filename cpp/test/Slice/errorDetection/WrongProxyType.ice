// **********************************************************************
//
// Copyright (c) 2001
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
//
// **********************************************************************

sequence<int> Seq;
dictionary<Seq, int> Dict;

interface I
{
    Seq* f1();
    void f2(Seq*);
    void f3(out Seq*);

    Dict* f4();
    void f5(Dict*);
    void f6(out Dict*);
    void op();
};

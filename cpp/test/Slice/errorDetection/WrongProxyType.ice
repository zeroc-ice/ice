// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

vector<int> Vector;
map<Vector, int> Map;

interface I
{
    Vector* f1();
    void f2(Vector*);
    void f3(; Vector*);

    Map* f1();
    void f2(Map*);
    void f3(; Map*);
};

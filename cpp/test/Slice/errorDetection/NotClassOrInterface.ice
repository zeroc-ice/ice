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
vector<int> Vector1;
vector<int> Vector2;
vector<int> Vector3;
interface BarIntf extends Vector { };
class BarClass1 extends Vector { };
class BarClass2 implements Vector1, Vector2, Vector3 { };
class BarClass3 extends Vector implements Vector1, Vector2, Vector3 { };

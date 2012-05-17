// **********************************************************************
//
// Copyright (c) 2003-2011 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

module Test
{

const byte C1 = 0;
const short C2 = 0;
const int C3 = -1;
const long C4 = 0x80000001;
const float C5 = 1.1;

enum E { e1, e2, e3 };

class C
{
    optional string m1;             // missing tag
    optional() int m2;              // missing tag
    optional(abc) bool m3;          // invalid tag
    optional(0x80000000) short m4;  // out of range
    optional(-0x80000001) long m5;  // out of range
    optional(-1) float m6;          // out of range
    optional(C1) string m7;         // ok
    optional(C2) string m8;         // duplicate tag
    optional(C3) double m9;         // out of range
    optional(C4) byte m10;          // out of range
    optional(C5) bool m11;          // invalid tag
    optional(e1) int m12;           // duplicate tag
};

};

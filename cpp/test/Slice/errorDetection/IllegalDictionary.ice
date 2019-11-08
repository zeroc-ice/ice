//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

module Test
{

dictionary<bool, long> d1;              // OK
dictionary<byte, long> d2;              // OK
dictionary<short, long> d3;             // OK
dictionary<int, long> d4;               // OK
dictionary<long, long> d5;              // OK
dictionary<string, long> d6;            // OK

dictionary<float, long> b1;             // Bad
dictionary<double, long> b2;            // Bad
dictionary<Object, long> b3;            // Bad
dictionary<Object*, long> b4;           // Bad

sequence<byte> s1;
dictionary<s1, long> d7;                // Deprecated

sequence<float> s2;
dictionary<s2, long> b6;                // Bad

sequence<s1> s3;
dictionary<s3, long> b7;                // Deprecated

struct st1
{
    long x;
    long y;
}
dictionary<st1, long> d8;               // OK

struct st2
{
    long x;
    float y;
}
dictionary<st2, long> b8;               // Bad

enum e { e1, e2 }
dictionary<e, long> d9;                 // OK

sequence<e> s4;
dictionary<s4, long> d10;               // Deprecated

dictionary<d9, long> b9;                // Bad

}

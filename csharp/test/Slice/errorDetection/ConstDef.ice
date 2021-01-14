//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

module Test
{

//
// Basic checks (all OK)
//
const bool boolconst = false;
const bool boolconst2 = true;
const byte byteconst = 0;
const short shortconst = 0;
const int intconst = 0;
const long longconst = 0;
const float floatconst = 0.;
const double doubleconst = 0.;
const string stringconst = "X\aX\x01000\x100\rX\007\xff\xff\xff\x0ff\xfff\x1X";
const string stringconst2 = "Hello World!";
enum color { red, green, blue }
const color colorconst = blue;
const ::Test::color colorconst2 = ::Test::color::green;

//
// Checks for number formats (all OK)
//
const long l1 = 127;
const long l2 = 0177;
const long l3 = 0x7f;

const float f1 = .1;
const float f2 = 1.1;
const float f3 = 1.;
const float f4 = .1e1;
const float f5 = 1.E1;
const float f6 = 1.1e+1;
const float f7 = 1.1e-1;
const float f8 = 1e-1;
const float f9 = 1E+1;
const float f10 = 1E+1f;
const float f11 = 1E1F;

const float pf1 = +.1;
const float pf2 = +1.1;
const float pf3 = +1.;
const float pf4 = +.1e1;
const float pf5 = +1.E1;
const float pf6 = +1.1e+1;
const float pf7 = +1.1e-1;
const float pf8 = +1e-1;
const float pf9 = +1E+1;
const float pf10 = +1E+1f;
const float pf11 = +1E1F;

const float nf1 = -.1;
const float nf2 = -1.1;
const float nf3 = -1.;
const float nf4 = -.1e1;
const float nf5 = -1.E1;
const float nf6 = -1.1e+1;
const float nf7 = -1.1e-1;
const float nf8 = -1e-1;
const float nf9 = -1E+1;
const float nf10 = -1E+1f;
const float nf11 = -1E1F;

const XXX ic1 = 1;              // no such type
const long f11 = 1;             // redefinition
const long F10 = 1;             // case-insensitive redefinition

sequence<long> LS;
const LS ic2 = 1;               // illegal constant type
const Object ic3 = 0;           // illegal constant type

const bool ic4 = 0;             // type mismatch
const byte ic5 = "hello";       // type mismatch
const short ic6 = 1.1;          // type mismatch
const int ic7 = blue;           // type mismatch
const long ic8 = false;         // type mismatch

const long = 0;                 // missing identifier

enum digit { one, two }
const color ic9 = two;          // wrong enumerator type
const color ic10 = 0;           // wrong initializer type

//
// Range checks
//
const long r1 = 9223372036854775807;    // LLONG_MAX, OK
const long r2 = -9223372036854775807;   // -LLONG_MAX, OK

//
// TODO: This should work, but doesn't. STLport can't handle inserting
// LLONG_MIN onto a stream and inserts garbage. We can periodically try
// this again as new versions of STLport come out...
//
// const long r3 = -9223372036854775808;        // LLONG_MIN, OK
//

const long r4 = -9223372036854775809;   // underflow
const long r5 = +9223372036854775808;   // overflow

const int i1 = -2147483648;             // INT_MIN, OK
const int i2 = +2147483647;             // INT_MAX, OK
const int i3 = -2147483649;             // underflow
const int i4 = +2147483648;             // overflow

const short s1 = -32768;                // SHORT_MIN, OK
const short s2 = +32767;                // SHORT_MAX, OK
const short s3 = -32769;                // underflow
const short s4 = +32768;                // overflow

const byte b1 = 0;                      // OK
const byte b2 = 255;                    // OK
const byte b3 = -1;                     // underflow
const byte b4 = 256;                    // overflow

const string nullstring1 = "a\000";     // OK
const string nullstring2 = "a\x000";    // OK
const string nullstring3 = "a\u0000";   // OK
const string nullstring4 = "a\U00000000"; // OK

const byte c1 = l1;             // OK
const short c2 = l1;            // OK
const int c3 = l1;              // OK
const long c4  = l1;            // OK

const byte c5 = s2;             // overflow
const short c6 = i2;            // overflow
const int c7 = r5;              // overflow

const float c8 = f1;            // OK
const float c9 = doubleconst;   // OK
const double c10 = f1;          // OK
const double c11 = doubleconst; // OK

const string c12 = stringconst; // OK

const color c13 = colorconst;   // OK

const string unknownEscape = "a\g";                  // Unknown escape sequence
const string invalidCodepoint = "a\u000N";          // Invalid code point
const string invalidCodepoint1 = "a\U0000000K";     // Invalid code point
const string octalRange = "\455";                   // Octal escape sequence out of range
const string badHex = "\x";                         // Missing hex digit
const string surrogatePair = "\uD83C\uDF4C";        // surrogate pair not allow in slice

}

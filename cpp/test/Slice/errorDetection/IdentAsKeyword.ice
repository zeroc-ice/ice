//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

module Test
{

interface INTERFACE { void op(); }

exception Void {}
exception int {}

struct OUT { long l; }
struct double { long l; }

struct s1 { long Int; }
struct s2 { long byte; }
struct s3 { byte b; short Int; }
struct s4 { byte b; float byte; }

class inTERface;
class interface;

class MOdule { long l; }
class module { long l; }

class C { long extendS; }
class C { long extends; }
class D { long extends; }

interface Object { void op(); }
interface object { void op(); }
interface long { void op(); }

sequence<long> impLEments;
sequence<long> implements;
sequence<long> short;

sequence<module> seq1;
sequence<moDule> seq2;

dictionary<long, long> throws;
dictionary<long, long> thRows;
dictionary<long, long> LOCALobject;

dictionary<module, long> d1;
dictionary<MODULE, long> d2;

dictionary<long, out> d3;
dictionary<long, OUT> d4;

dictionary<void, void> d5;
dictionary<VOID, VOID> d6;

enum e1 { long, byte, foo }
enum e2 { LONG, BYTE, bar } // Ok as of 3.7

interface i1 { long module(); }
interface i2 { long mODule(); }

interface i3 { void exception(); }
interface i4 { void EXception(); }

interface i5 { out op(); }
interface i6 { OUT op(); }

interface i7 { void op(double byte); }
interface i8 { void op(double BYTE); }

interface i9 { void op(out double byte); }
interface i10 { void op(out double BYTE); }

interface i11 { enum op(); }
interface i12 { out enum op(); }

interface \true {}     // OK, escaped keyword

}

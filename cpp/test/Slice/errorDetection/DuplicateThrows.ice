exception e {};
exception e2 {};
interface i2 { void op() throws ::e, e; };
interface i3 { void op() throws e, ::e; };
interface i4 { void op() throws e2, e, e2; };
interface i5 { void op() throws e2, e, e2, ::e; };

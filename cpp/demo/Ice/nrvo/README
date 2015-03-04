A simple demo that shows how Ice takes advantage of NRVO (Named
Return Value Optimization), which is implemented by modern C++
compilers.

GCC and Microsoft Visual Studio compilers support this optimization.

You can read more about Visual Studio NRVO at:

  * http://msdn.microsoft.com/en-us/library/ms364057(VS.80).aspx

For GCC, NRVO was first introduced in version 3.1. See the release
changes at:

  * http://www.gnu.org/software/gcc/gcc-3.1/changes.html

To run the demo, first start the server:

$ server

In a separate window, start the client:

$ client

The client presents a menu of options that you can invoke by pressing
the corresponding key. All of these operations return a string
sequence mapped to the custom class MyStringSeq. This class prints
the message "MyStringSeq copy ctor" each time it is copied so that you
can see when copies of the returned data are made by the Ice run time.

With Visual C++, you must compile with optimization for NRVO to be
enabled. GCC applies NRVO whether the code is optimized or not.

In the server, we show one case when NRVO works: a function that has a
single return path implemented in NrvoI::op1. There are also two cases
where NRVO doesn't work: a function that returns a data member of
the servant (NrvoI::op2) and a function with multiple return paths
(NrvoI::op3).

Invoking op1 demonstrates that NRVO is applied in both the client and
the server, whereas for op2 and op3 NRVO is only applied in the
client.

The client can also invoke op3 such that RVO (Return Value Optimization)
is applied for a return statement with in-place construction. (See
http://www.efnetcpp.org/wiki/Return_value_optimization for information
on RVO.)

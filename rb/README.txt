Ice for Ruby Preview Release
----------------------------

The archive you downloaded contains the Ice extension for Ruby, along
with supporting libraries and sample programs.

Requirements:

* Ice 3.1.1 for Microsoft Visual C++ 6.0
* Ruby 1.8.4 or 1.8.5

You can obtain a Windows installer for Ice 3.1.1 from the ZeroC
web site:

http://www.zeroc.com/download.html

An installer for Ruby is available here:

http://rubyforge.org/projects/rubyinstaller/


Installation Instructions
-------------------------

The steps below assume you have extracted the Ice for Ruby archive
into the C:\IceRuby-3.1.1 directory.

1. Add C:\IceRuby-3.1.1\bin to your PATH.

2. Verify that the Ice run time is also in your PATH. For example, if
   Ice is installed in C:\Ice, then PATH should include the C:\Ice\bin
   directory.

3. Define the RUBYLIB environment variable as follows:

   set RUBYLIB=C:\IceRuby-3.1.1\ruby

   Note that there are other ways of informing the Ruby interpreter
   about the location of the Ice extension and supporting files. See
   the accompanying Ice for Ruby documentation for more information.


Running the demos
-----------------

The demos directory contains Ruby versions of the Ice sample programs.
Note that only clients are provided, since Ice for Ruby does not
support server-side activities. In order to run a sample client, you
must first start its corresponding server from another Ice language
mapping, such as C++.

As an example, let's run the hello application in demo\Ice\hello using
the C++ server. Assuming that your Ice installation is located in
C:\Ice, and that you have already compiled the sample C++ programs,
we begin by starting the server:

> cd \Ice\demo\Ice\hello
> server

In a separate window, start the client:

> cd \IceRuby-3.1.1\demo\Ice\hello
> ruby Client.rb

Some demo directories contain README files if additional requirements
are necessary.

Ice for Ruby Preview Release
----------------------------

This binary archive contains the Ice extension for Ruby, supporting
libraries and sample programs. The executables and DLLs in this
archive were compiled with Microsoft Visual C++ 6.0 for compatibility
with Ruby.

Requirements:

- Ice for C++ 3.1.1 run time (Microsoft Visual C++ 6.0)
- Ruby 1.8.5

You can obtain a Windows installer for Ice 3.1.1 from the ZeroC
web site:

  http://www.zeroc.com/download.html

An installer for Ruby is available here:

  http://rubyforge.org/projects/rubyinstaller/


Installation Instructions
-------------------------

The steps below assume you have extracted this Ice for Ruby archive
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

4. Verify that Ruby can load the Ice extension successfully. Open a
   command window and start the interpreter using irb.bat, as shown
   below:

   > irb.bat
   irb(main):001:0>

   At the prompt, enter

   require "Ice"

   If the interpreter responds with the value "true", the Ice
   extension was loaded successfully. Enter "exit" to quit the
   interpreter.


Limitations
-----------

Ruby's lack of support for native threads makes it impractical for
ZeroC to implement server-side functionality, therefore Ice for Ruby
can only be used in client applications. If you have a need for this
feature and wish to sponsor its development, please contact us at
info@zeroc.com.

Ice for Ruby currently does not support the thread-per-connection
concurrency model on Windows.


OpenSSL Compatibility Issue
---------------------------

The Ruby installer includes versions of the OpenSSL DLLs that are not
compatible with the ones supplied with Ice. If you intend to use SSL
in your Ice for Ruby applications, you will need to remove or rename
the following files in the Ruby installation directory:

  libeay32.dll
  ssleay32.dll

If you used the default installation directory, these files are
located in C:\ruby\bin.

Also be aware that the Ruby installer inserts C:\ruby\bin at the
beginning of the system PATH, therefore the DLLs listed above can also
have an adverse impact on other Ice language mappings that use
OpenSSL, such as C++ and Python.


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

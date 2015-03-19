This is a test for the string converter implementations.
By default, it uses the Ice::WindowsStringConverter on Windows and the 
Ice::IconvStringConverter on other platforms.

If you want to use iconv on Windows, set ICONV_HOME to your Iconv 
installation, and put %ICONV_HOME%\bin in your PATH when running 
the test suite.

The Windows build system assumes that your Iconv DLL uses its own
C runtime library: therefore it's not possible to retrieve Iconv's
errno and we build with -DICE_NO_ERRNO.

Remove this -DICE_NO_ERRNO if you have a better iconv build.

// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <TestCommon.h>
#include <Test.h>

#include <iostream>
#include <locale.h>

using namespace std;

class Client : public Ice::Application
{
public:

    virtual int run(int, char*[]);
};

static bool useLocale = false;
static bool useIconv = true;

int
main(int argc, char* argv[])
{
#ifdef ICE_STATIC_LIBS
    Ice::registerIceSSL();
#endif

    Client app;

#ifndef _WIN32
    //
    // Switch to French locale
    // (we just used the codeset for as default internal code for
    // stringConverter below)
    //
    useLocale = (setlocale(LC_ALL, "fr_FR.ISO8859-15") != 0 || setlocale(LC_ALL, "fr_FR.iso885915@euro") != 0);
#endif

#if defined(_WIN32)
    //
    // 28605 == ISO 8859-15 codepage
    //
    IceUtil::setProcessStringConverter(new IceUtil::WindowsStringConverter(28605));
    useIconv = false;

#elif defined(__hpux)
    if(useLocale)
    {
        IceUtil::setProcessStringConverter(new IceUtil::IconvStringConverter<char>);
    }
    else
    {
        IceUtil::setProcessStringConverter(new IceUtil::IconvStringConverter<char>("iso815"));
    }
    IceUtil::setProcessWstringConverter(new IceUtil::IconvStringConverter<wchar_t>("ucs4"));
    
#elif defined(_AIX)
    
    // Always big-endian
    
    if(useLocale)
    {
	IceUtil::setProcessStringConverter(new IceUtil::IconvStringConverter<char>());
    }
    else
    {
        IceUtil::setProcessStringConverter(new IceUtil::IconvStringConverter<char>("ISO8859-15"));
    }

    if(sizeof(wchar_t) == 4)
    {
	IceUtil::setProcessWstringConverter(new IceUtil::IconvStringConverter<wchar_t>("UTF-32"));
    }
    else
    {  
	IceUtil::setProcessWstringConverter(new IceUtil::IconvStringConverter<wchar_t>("UTF-16"));
    }    
#else

    if(useLocale)
    {
        IceUtil::setProcessStringConverter(new IceUtil::IconvStringConverter<char>());
    }
    else
    {
        IceUtil::setProcessStringConverter(new IceUtil::IconvStringConverter<char>("ISO8859-15"));
    }

    if(sizeof(wchar_t) == 4)
    {
#  ifdef ICE_BIG_ENDIAN
        IceUtil::setProcessWstringConverter(new IceUtil::IconvStringConverter<wchar_t>("UTF-32BE"));
#  else
        IceUtil::setProcessWstringConverter(new IceUtil::IconvStringConverter<wchar_t>("UTF-32LE"));
#  endif
    }
    else
    {
#  ifdef ICE_BIG_ENDIAN
        IceUtil::setProcessWstringConverter(new IceUtil::IconvStringConverter<wchar_t>("UTF-16BE"));
#  else
        IceUtil::setProcessWstringConverter(new IceUtil::IconvStringConverter<wchar_t>("UTF-16LE"));
#  endif
    }
#endif
    return app.main(argc, argv);
}

int
Client::run(int, char*[])
{
    Test::MyObjectPrxPtr proxy =
        ICE_UNCHECKED_CAST(Test::MyObjectPrx,
                           communicator()->stringToProxy("test:" + getTestEndpoint(communicator(), 0)));

    char oe = char(0xBD); // A single character in ISO Latin 9
    string msg = string("tu me fends le c") + oe + "ur!";
    cout << "testing string converter";
    if(useLocale)
    {
        cout << " (using locale)";
    }
    if(useIconv)
    {
        cout << " (using iconv)";
    }
    cout << "... " << flush;
    wstring wmsg = proxy->widen(msg);
    test(proxy->narrow(wmsg) == msg);
    test(wmsg.size() == msg.size());
    cout << "ok" << endl;
    proxy->shutdown();
    return EXIT_SUCCESS;
}

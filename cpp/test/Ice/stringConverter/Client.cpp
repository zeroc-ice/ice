// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
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

DEFINE_TEST("client")

using namespace std;

static bool useLocale = false;
static bool useIconv = true;

int
main(int argc, char* argv[])
{
#ifdef ICE_STATIC_LIBS
    Ice::registerIceSSL(false);
    Ice::registerIceWS(true);
    Ice::registerIceStringConverter(false);
#endif

    Ice::InitializationData initData = getTestInitData(argc, argv);

    string narrowEncoding;
    string wideEncoding;

#ifdef _WIN32
    useIconv = false;
#else
    //
    // Switch to French locale
    // (we just used the codeset for as default internal code for
    // stringConverter below)
    //
    useLocale = (setlocale(LC_ALL, "fr_FR.ISO8859-15") != 0 || setlocale(LC_ALL, "fr_FR.iso885915@euro") != 0);
#endif

    if(useIconv)
    {

#if defined(__hpux)
        narrowEncoding = "iso815";
        wideEncoding = "ucs4";

#elif defined(_AIX)

        // Always big-endian
        narrowEncoding = "ISO8859-15";

        if(sizeof(wchar_t) == 4)
        {
            wideEncoding = "UTF-32";
        }
        else
        {
            wideEncoding = "UTF-16";
        }
#else

        narrowEncoding = "ISO8859-15";

        if(sizeof(wchar_t) == 4)
        {
#  ifdef ICE_BIG_ENDIAN
            wideEncoding = "UTF-32BE";
#  else
            wideEncoding = "UTF-32LE";
#  endif
        }
        else
        {
#  ifdef ICE_BIG_ENDIAN
            wideEncoding = "UTF-16BE";
#  else
            wideEncoding = "UTF-16LE";
#  endif
        }
#endif
    }

    {
#ifdef _WIN32
        //
        // 28605 == ISO 8859-15 codepage
        //
        setProcessStringConverter(Ice::createWindowsStringConverter(28605));
#else
        if(useLocale)
        {
            setProcessStringConverter(Ice::createIconvStringConverter<char>(""));
        }
        else
        {
            setProcessStringConverter(Ice::createIconvStringConverter<char>(narrowEncoding));
        }

        setProcessWstringConverter(Ice::createIconvStringConverter<wchar_t>(wideEncoding));

#endif

        Ice::CommunicatorPtr communicator = Ice::initialize(initData);
        Test::MyObjectPrxPtr proxy =
            ICE_UNCHECKED_CAST(Test::MyObjectPrx,
                               communicator->stringToProxy("test:" + getTestEndpoint(communicator, 0)));

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

        // Test stringToIdentity and identityToString

        string identStr = "cat/" + msg;
        Ice::Identity ident = Ice::stringToIdentity(identStr);
        test(ident.name == msg);
        test(ident.category == "cat");
        test(identityToString(ident, Ice::ICE_ENUM(ToStringMode, Unicode)) == identStr);

        identStr = identityToString(ident, Ice::ICE_ENUM(ToStringMode, ASCII));
        test(identStr == "cat/tu me fends le c\\u0153ur!");
        test(Ice::stringToIdentity(identStr) == ident);
        identStr = identityToString(ident, Ice::ICE_ENUM(ToStringMode, Compat));
        test(identStr == "cat/tu me fends le c\\305\\223ur!");
        test(Ice::stringToIdentity(identStr) == ident);

        communicator->destroy();
        cout << "ok" << endl;
    }

    Ice::setProcessStringConverter(ICE_NULLPTR);
    Ice::setProcessWstringConverter(Ice::createUnicodeWstringConverter());

    string propValue = "Ice:createStringConverter";
    if(useIconv && !useLocale)
    {
        propValue +=  " iconv=" + narrowEncoding + "," + wideEncoding;
    }
    propValue += " windows=28605";

    initData.properties->setProperty("Ice.Plugin.IceStringConverter", propValue);

    Ice::CommunicatorPtr communicator = Ice::initialize(initData);
    Test::MyObjectPrxPtr proxy =
        ICE_UNCHECKED_CAST(Test::MyObjectPrx,
                           communicator->stringToProxy("test:" + getTestEndpoint(communicator, 0)));

    char oe = char(0xBD); // A single character in ISO Latin 9
    string msg = string("tu me fends le c") + oe + "ur!";
    cout << "testing string converter plug-in";
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
    communicator->destroy();

    return EXIT_SUCCESS;
}

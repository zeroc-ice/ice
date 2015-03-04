// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <TestCommon.h>
#include <Test.h>

#if defined(ICONV_ON_WINDOWS)
//
// On Windows, Ice/IcongStringConverter.h is not included by Ice/Ice.h
//
#include <Ice/IconvStringConverter.h>
#endif

#include <iostream>
#include <locale.h>

using namespace std;

class Client : public Ice::Application
{
public:

    virtual int run(int, char*[]);
};

//
// Server side is pure unicode
//
class MyObjectI : public Test::MyObject
{
public:

    virtual wstring widen(const string& msg, const Ice::Current&)
    {
        const Ice::Byte* cmsg = reinterpret_cast<const Ice::Byte*>(msg.c_str());

        if(!IceUtil::isLegalUTF8Sequence(cmsg, cmsg + msg.size()))
        {
            throw Test::BadEncodingException();
        }

        return IceUtil::stringToWstring(msg);
    }
    
    virtual string narrow(const wstring& wmsg, const Ice::Current&)
    {
        return IceUtil::wstringToString(wmsg);
    }
};

static bool useLocale = false;
static bool useIconv = true;

int
main(int argc, char* argv[])
{
    Client app;

#ifndef _WIN32
    //
    // Switch to French locale
    // (we just used the codeset for as default internal code for 
    // initData.stringConverter below)
    //

    useLocale = (setlocale(LC_ALL, "fr_FR.ISO8859-15") != 0
                 || setlocale(LC_ALL, "fr_FR.iso885915@euro") != 0);
#endif
   
    Ice::InitializationData initData;

#if defined(_WIN32) && !defined(ICONV_ON_WINDOWS)
    //
    // 28605 == ISO 8859-15 codepage
    //
    initData.stringConverter = new Ice::WindowsStringConverter(28605);
    useIconv = false;

#elif defined(__hpux)
    if(useLocale)
    {
        initData.stringConverter = new Ice::IconvStringConverter<char>;
    }
    else
    {
        initData.stringConverter = new Ice::IconvStringConverter<char>("iso815");
    }
    initData.wstringConverter = new Ice::IconvStringConverter<wchar_t>("ucs4");  
#else
    
    if(useLocale)
    {
#ifndef _WIN32
        initData.stringConverter = new Ice::IconvStringConverter<char>;
#endif
    }
    else
    {
        initData.stringConverter = new Ice::IconvStringConverter<char>("ISO8859-15");
    }

    if(sizeof(wchar_t) == 4)
    {
#ifdef ICE_BIG_ENDIAN
        initData.wstringConverter = new Ice::IconvStringConverter<wchar_t>("UTF-32BE");
#else
        initData.wstringConverter = new Ice::IconvStringConverter<wchar_t>("UTF-32LE");
#endif
    }
    else
    {
#ifdef ICE_BIG_ENDIAN
        initData.wstringConverter = new Ice::IconvStringConverter<wchar_t>("UTF-16BE");
#else
        initData.wstringConverter = new Ice::IconvStringConverter<wchar_t>("UTF-16LE");
#endif
    }
#endif
    return app.main(argc, argv, initData);
}

int
Client::run(int, char*[])
{
    //
    // Create server communicator and OA
    //
    Ice::InitializationData initData;
    initData.properties = communicator()->getProperties()->clone();
    Ice::CommunicatorPtr serverCommunicator = Ice::initialize(initData);
    Ice::ObjectAdapterPtr oa = serverCommunicator->createObjectAdapterWithEndpoints("MyOA", "tcp -h localhost");
    
    Ice::ObjectPtr servant = new MyObjectI;
    Test::MyObjectPrx serverPrx = Test::MyObjectPrx::uncheckedCast(oa->addWithUUID(servant));
    oa->activate();

    //
    // Get a prx in the client's communicator
    //
    Test::MyObjectPrx clientPrx = 
        Test::MyObjectPrx::uncheckedCast(communicator()->stringToProxy(serverPrx->ice_toString()));

    char oe =  char(0xBD); // A single character in ISO Latin 9
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
    wstring wmsg = clientPrx->widen(msg);
    test(clientPrx->narrow(wmsg) == msg);
    test(wmsg.size() == msg.size());
    cout << "ok" << endl;

    //
    // destroy server communicator
    //
    serverCommunicator->destroy();
    return EXIT_SUCCESS;
}

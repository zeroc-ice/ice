// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <TestCommon.h>
#include <Test.h>
#include <Ice/IconvStringConverter.h>
#include <iostream>

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

int
main(int argc, char* argv[])
{
    Client app;
    
    Ice::InitializationData initData;

#if defined(__hpux)
    initData.stringConverter = new Ice::IconvStringConverter<char>("iso815");
    initData.wstringConverter = new Ice::IconvStringConverter<wchar_t>("ucs4");  
#else   
    initData.stringConverter = new Ice::IconvStringConverter<char>("ISO-8859-15");
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
    Ice::CommunicatorPtr serverCommunicator = Ice::initialize();
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
    cout << "testing iconv string converter..." << flush;
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

//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Communicator.h>
#include <ImplicitContext.h>
#include <Logger.h>
#include <Operation.h>
#include <Properties.h>
#include <Proxy.h>
#include <Slice.h>
#include <Types.h>
#include <Connection.h>
#include <Endpoint.h>
#include <ValueFactoryManager.h>

#include <Ice/RegisterPlugins.h>

using namespace std;
using namespace IceRuby;

static VALUE iceModule;

extern "C"
{

void
ICE_DECLSPEC_EXPORT Init_IceRuby()
{
    Ice::registerIceSSL(false);
    Ice::registerIceDiscovery(false);
    Ice::registerIceLocatorDiscovery(false);

    iceModule = rb_define_module("Ice");
    initCommunicator(iceModule);
    initLogger(iceModule);
    initOperation(iceModule);
    initProperties(iceModule);
    initProxy(iceModule);
    initSlice(iceModule);
    initTypes(iceModule);
    initImplicitContext(iceModule);
    initUtil(iceModule);
    initConnection(iceModule);
    initEndpoint(iceModule);
    initValueFactoryManager(iceModule);
}

}

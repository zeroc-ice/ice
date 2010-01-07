// **********************************************************************
//
// Copyright (c) 2003-2010 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef TEST_CONFIGURATION_H
#define TEST_CONFIGURATION_H

#include <IceUtil/Shared.h>
#include <IceUtil/Handle.h>

#include <Ice/LocalException.h>
#include <Ice/Selector.h>
#include <memory>

class Configuration;
typedef IceUtil::Handle<Configuration> ConfigurationPtr;

class Configuration : public IceUtil::Shared, public IceUtil::Mutex
{
public:

    Configuration();
    virtual ~Configuration();

    virtual void connectorsException(Ice::LocalException*);
    virtual void checkConnectorsException();

    virtual void connectException(Ice::LocalException*);
    virtual void checkConnectException();

    virtual void initializeSocketOperation(IceInternal::SocketOperation);
    virtual void initializeException(Ice::LocalException*);
    virtual IceInternal::SocketOperation initializeSocketOperation();
    virtual void checkInitializeException();

    virtual void readReady(bool);
    virtual void readException(Ice::LocalException*);
    virtual bool readReady();
    virtual void checkReadException();

    virtual void writeReady(bool);
    virtual void writeException(Ice::LocalException*);
    virtual bool writeReady();
    virtual void checkWriteException();

    static Configuration* getInstance();

private:

    std::auto_ptr<Ice::LocalException> _connectorsException;
    std::auto_ptr<Ice::LocalException> _connectException;
    IceInternal::SocketOperation _initializeSocketOperation;
    int _initializeResetCount;
    std::auto_ptr<Ice::LocalException> _initializeException;
    int _readReadyCount;
    std::auto_ptr<Ice::LocalException> _readException;
    int _writeReadyCount;
    std::auto_ptr<Ice::LocalException> _writeException;

    static Configuration* _instance;
};

#endif

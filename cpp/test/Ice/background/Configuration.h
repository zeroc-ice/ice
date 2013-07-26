// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef TEST_CONFIGURATION_H
#define TEST_CONFIGURATION_H

#include <IceUtil/IceUtil.h>

#include <Ice/LocalException.h>
#include <Ice/Selector.h>

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

    IceUtil::UniquePtr<Ice::LocalException> _connectorsException;
    IceUtil::UniquePtr<Ice::LocalException> _connectException;
    IceInternal::SocketOperation _initializeSocketOperation;
    int _initializeResetCount;
    IceUtil::UniquePtr<Ice::LocalException> _initializeException;
    int _readReadyCount;
    IceUtil::UniquePtr<Ice::LocalException> _readException;
    int _writeReadyCount;
    IceUtil::UniquePtr<Ice::LocalException> _writeException;

    static Configuration* _instance;
};

#endif

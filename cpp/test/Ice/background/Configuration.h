//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef TEST_CONFIGURATION_H
#define TEST_CONFIGURATION_H

#include <IceUtil/IceUtil.h>

#include <Ice/LocalException.h>
#include <Ice/Selector.h>

#ifndef TEST_API
#    if defined(ICE_STATIC_LIBS)
#        define TEST_API /**/
#    elif defined(TEST_API_EXPORTS)
#        define TEST_API ICE_DECLSPEC_EXPORT
#    else
#        define TEST_API ICE_DECLSPEC_IMPORT
#    endif
#endif

class Configuration;
using ConfigurationPtr = std::shared_ptr<Configuration>;

class TEST_API Configuration : public std::enable_shared_from_this<Configuration>
{
public:
    Configuration();
    void init();

    virtual ~Configuration();

    void connectorsException(Ice::LocalException*);
    void checkConnectorsException();

    void connectException(Ice::LocalException*);
    void checkConnectException();

    void initializeSocketOperation(IceInternal::SocketOperation);
    void initializeException(Ice::LocalException*);
    IceInternal::SocketOperation initializeSocketOperation();
    void checkInitializeException();

    void readReady(bool);
    void readException(Ice::LocalException*);
    bool readReady();
    void checkReadException();

    void writeReady(bool);
    void writeException(Ice::LocalException*);
    bool writeReady();
    void checkWriteException();

    void buffered(bool);
    bool buffered();

    static ConfigurationPtr getInstance();

private:
    std::unique_ptr<Ice::LocalException> _connectorsException;
    std::unique_ptr<Ice::LocalException> _connectException;
    IceInternal::SocketOperation _initializeSocketOperation;
    int _initializeResetCount;
    std::unique_ptr<Ice::LocalException> _initializeException;
    int _readReadyCount;
    std::unique_ptr<Ice::LocalException> _readException;
    int _writeReadyCount;
    std::unique_ptr<Ice::LocalException> _writeException;
    bool _buffered;
    std::mutex _mutex;
    static ConfigurationPtr _instance;
};

#endif

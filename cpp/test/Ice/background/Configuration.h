// Copyright (c) ZeroC, Inc.

#ifndef TEST_CONFIGURATION_H
#define TEST_CONFIGURATION_H

#include "Ice/Ice.h"
#include "Ice/Network.h"

#ifndef TEST_API
#    if defined(TEST_API_EXPORTS)
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

    void connectorsException(std::exception_ptr);
    void checkConnectorsException();

    void connectException(std::exception_ptr);
    void checkConnectException();

    void initializeSocketOperation(IceInternal::SocketOperation);
    void initializeException(std::exception_ptr);
    IceInternal::SocketOperation initializeSocketOperation();
    void checkInitializeException();

    void readReady(bool);
    void readException(std::exception_ptr);
    bool readReady();
    void checkReadException();

    void writeReady(bool);
    void writeException(std::exception_ptr);
    bool writeReady();
    void checkWriteException();

    void buffered(bool);
    bool buffered();

    static ConfigurationPtr getInstance();

private:
    std::exception_ptr _connectorsException;
    std::exception_ptr _connectException;
    IceInternal::SocketOperation _initializeSocketOperation{IceInternal::SocketOperationNone};
    int _initializeResetCount{0};
    std::exception_ptr _initializeException;
    int _readReadyCount{0};
    std::exception_ptr _readException;
    int _writeReadyCount{0};
    std::exception_ptr _writeException;
    bool _buffered{false};
    std::mutex _mutex;
    static ConfigurationPtr _instance;
};

#endif

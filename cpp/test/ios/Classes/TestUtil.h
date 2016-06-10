// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef TEST_UTIL_H
#define TEST_UTIL_H

#include <TestHelper.h>
#include <UIKit/UIKit.h>
#include <Foundation/NSString.h>

@interface TestCase : NSObject
{
@private

    NSString* name;
    NSString* prefix;
    NSString* client;
    NSString* server;
    NSString* serveramd;
    NSString* collocated;
    BOOL sslSupport;
    BOOL wsSupport;
    BOOL runWithSlicedFormat;
    BOOL runWith10Encoding;
    BOOL cpp11Support;
}

+(id) testWithName:(NSString*)name
                 prefix:(NSString*) prefix
                 client:(NSString*) client
                 server:(NSString*) server
              serveramd:(NSString*) serveramd
             collocated:(NSString*) collocated
             sslSupport:(BOOL) sslSupport
              wsSupport:(BOOL) wsSupport
    runWithSlicedFormat:(BOOL)runWithSlicedFormat
      runWith10Encoding:(BOOL)runWith10Encoding
           cpp11Support:(BOOL)cpp11Support;

-(BOOL)hasServer;
-(BOOL)hasAMDServer;
-(BOOL)hasCollocated;
-(BOOL)isProtocolSupported:(NSString*)protocol;

@property (readonly) NSString* name;
@property (readonly) NSString* prefix;
@property (readonly) NSString* client;
@property (readonly) NSString* server;
@property (readonly) NSString* serveramd;
@property (readonly) NSString* collocated;
@property (readonly) BOOL sslSupport;
@property (readonly) BOOL wsSupport;
@property (readonly) BOOL runWithSlicedFormat;
@property (readonly) BOOL runWith10Encoding;
@property (readonly) BOOL cpp11Support;

@end

typedef int (*MAIN_ENTRY_POINT)(int argc, char** argv, Test::MainHelper* helper);
typedef int (*SHUTDOWN_ENTRY_POINT)();

enum TestConfigType { TestConfigTypeClient, TestConfigTypeServer, TestConfigTypeColloc };
enum TestConfigOption {TestConfigOptionDefault, TestConfigOptionSliced, TestConfigOptionEncoding10 };

struct TestConfig
{
    TestConfigType type;
    TestConfigOption option;
    std::string protocol;
    bool hasServer;
};

class MainHelperI : public Test::MainHelper
{
public:

    MainHelperI(const std::string& test, const std::string& libName, TestConfig config,
                id target, SEL output, SEL ready) :
        _test(test),
        _libName(libName),
        _config(config),
        _completed(false),
        _status(0),
        _target(target),
        _output(output),
        _ready(ready)
    {
    }

    virtual ~MainHelperI();

    virtual void run();

    virtual void serverReady();

    virtual void shutdown();

    virtual void waitForCompleted(){};

    virtual bool redirect();

    virtual void print(const std::string&);

    int
    status()
    {
        return _status;
    }

private:

    void
    completed(int status)
    {
        _completed = true;
        _status = status;
    }

    std::string _test;
    std::string _libName;
    TestConfig _config;
    CFBundleRef _handle;
    SHUTDOWN_ENTRY_POINT _dllTestShutdown;
    bool _completed;
    int _status;

    id _target;
    SEL _ready;
    SEL _output;
};

#endif

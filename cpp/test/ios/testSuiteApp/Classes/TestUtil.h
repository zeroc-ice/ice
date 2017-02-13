// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
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

#include <vector>

@interface TestSuite : NSObject
{
@private
    NSString* testSuiteId;
    NSArray* testCases;
}
@property (nonatomic, retain) NSString* testSuiteId;
@property (nonatomic, retain) NSArray* testCases;
+(id) testSuite:(NSString*)testSuiteId testCases:(NSArray*)testCases;
-(BOOL) isIn:(NSArray*)tests;
@end

@interface TestCase : NSObject
{
@private
    NSString* name;
    NSString* client;
    NSString* server;
    NSArray* args;
}
+(id) testCase:(NSString*)name client:(NSString*)client server:(NSString*)server args:(NSArray*)args;
@property (nonatomic, retain) NSString* name;
@property (nonatomic, retain) NSString* client;
@property (nonatomic, retain) NSString* server;
@property (nonatomic, retain) NSArray* args;
@end

typedef int (*MAIN_ENTRY_POINT)(int argc, char** argv, Test::MainHelper* helper);
typedef int (*SHUTDOWN_ENTRY_POINT)();

enum TestType { TestTypeClient, TestTypeServer };

struct TestConfig
{
    std::string protocol;
    std::vector<std::string> args;
};

class MainHelperI : public Test::MainHelper
{
public:

    MainHelperI(const std::string&, const std::string&, TestType, TestConfig, id, SEL, SEL);
    virtual ~MainHelperI();

    virtual void run();
    virtual void serverReady();
    virtual void shutdown();
    virtual void waitForCompleted() { };
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

    std::string _name;
    std::string _libName;
    TestType _type;
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

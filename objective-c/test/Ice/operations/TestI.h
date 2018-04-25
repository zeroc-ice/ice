// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#import <OperationsTest.h>
#import <Foundation/Foundation.h>

//
// Servant implementation
//
@interface TestOperationsMyDerivedClassI : TestOperationsMyDerivedClass<TestOperationsMyDerivedClass>
{
    int _opByteSOnewayCallCount;
    NSCondition* _cond;
}
@end

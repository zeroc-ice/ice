// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
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

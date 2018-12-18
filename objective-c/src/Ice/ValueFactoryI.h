// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#import <objc/Ice/ValueFactory.h>

#include <Ice/Communicator.h>

@interface ICEValueFactoryManager : NSObject<ICEValueFactoryManager>
{
    NSMutableDictionary* valueFactories_;
}
-(id) init:(Ice::Communicator*)communicator prefixTable:(NSDictionary*)prefixTable;
@end

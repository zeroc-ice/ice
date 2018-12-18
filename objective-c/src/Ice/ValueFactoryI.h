// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
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

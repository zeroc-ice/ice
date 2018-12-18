// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

#import <objc/Ice/Communicator.h>
#import <objc/Ice/LocalObject.h>

#import <Foundation/NSSet.h>

#include <Ice/Communicator.h>

@class ICEObjectAdapter;
@class ICEValueFactoryManager;
@class ICEInitializationData;

@interface ICECommunicator : ICELocalObject<ICECommunicator>
{
    NSDictionary* prefixTable_;
    NSMutableDictionary* adminFacets_;
    ICEValueFactoryManager* valueFactoryManager_;
    NSMutableDictionary* objectFactories_;
}
-(void)setup:(ICEInitializationData*)prefixTable;
-(Ice::Communicator*)communicator;
-(NSDictionary*)getPrefixTable;
@end

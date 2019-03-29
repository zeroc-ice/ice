// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#import "IceObjcException.h"

@implementation ICERuntimeException
@synthesize file;
@synthesize line;
@end

@implementation ICERequestFailedException
@synthesize name;
@synthesize category;
@synthesize facet;
@synthesize operation;
@end

@implementation ICEObjectNotExistException
@end

@implementation ICEFacetNotExistException
@end

@implementation ICEOperationNotExistException
@end

@implementation ICEUnknownException
@synthesize unknown;
@end

@implementation ICEUnknownLocalException
@end

@implementation ICEUnknownUserException
@end

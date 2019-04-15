//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

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

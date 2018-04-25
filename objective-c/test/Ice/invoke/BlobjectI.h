// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#import <InvokeTest.h>

@interface BlobjectI : ICEBlobject<ICEBlobject>
-(BOOL) ice_invoke:(NSData*)inEncaps outEncaps:(NSMutableData**)outEncaps current:(ICECurrent*)current;
@end

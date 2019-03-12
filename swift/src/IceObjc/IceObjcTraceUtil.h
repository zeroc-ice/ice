// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

NS_ASSUME_NONNULL_BEGIN

@interface ICETraceUtil : NSObject

+(void) traceSlicing:(NSString*)kind
              typeId:(NSString*)typeId
          slicingCat:(NSString*)slicingCat
              logger:(id<ICELoggerProtocol>)logger NS_SWIFT_NAME(traceSlicing(kind:typeId:slicingCat:logger:));

@end

NS_ASSUME_NONNULL_END

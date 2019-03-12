// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#import "IceObjcLogger.h"
#import "IceObjcTraceUtil.h"

#import <Ice/TraceUtil.h>
#import "LoggerWrapperI.h"

@implementation ICETraceUtil

+(void) traceSlicing:(NSString*)kind
              typeId:(NSString*)typeId
          slicingCat:(NSString*)slicingCat
logger:(id<ICELoggerProtocol>)logger
{
    auto l = std::make_shared<LoggerWrapperI>(logger);
    IceInternal::traceSlicing(fromNSString(kind).c_str(), fromNSString(typeId), fromNSString(slicingCat).c_str(), l);
}

@end

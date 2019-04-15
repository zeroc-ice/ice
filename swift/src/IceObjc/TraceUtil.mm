//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

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

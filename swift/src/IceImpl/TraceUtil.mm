// Copyright (c) ZeroC, Inc.

#import "include/TraceUtil.h"

#import "LoggerWrapperI.h"

// Redeclare internal function from the Ice C++ library.
namespace IceInternal
{
    void traceSlicing(const char*, std::string_view, const char*, const Ice::LoggerPtr&);
}

@implementation ICETraceUtil

+ (void)traceSlicing:(NSString*)kind
              typeId:(NSString*)typeId
          slicingCat:(NSString*)slicingCat
              logger:(id<ICELoggerProtocol>)logger
{
    auto l = std::make_shared<LoggerWrapperI>(logger);
    IceInternal::traceSlicing(fromNSString(kind).c_str(), fromNSString(typeId), fromNSString(slicingCat).c_str(), l);
}

@end

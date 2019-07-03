//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

NS_ASSUME_NONNULL_BEGIN

ICEOBJC_API @interface ICETraceUtil : NSObject

+(void) traceSlicing:(NSString*)kind
              typeId:(NSString*)typeId
          slicingCat:(NSString*)slicingCat
              logger:(id<ICELoggerProtocol>)logger NS_SWIFT_NAME(traceSlicing(kind:typeId:slicingCat:logger:));

@end

NS_ASSUME_NONNULL_END

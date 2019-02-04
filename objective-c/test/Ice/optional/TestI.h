//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#import <OptionalTest.h>

@interface InitialI : TestOptionalInitial<TestOptionalInitial>
-(void) shutdown:(ICECurrent *)current;
-(ICEObject*) pingPong:(ICEObject*)obj current:(ICECurrent*)current;
-(BOOL) supportsRequiredParams:(ICECurrent*)current;
-(BOOL) supportsJavaSerializable:(ICECurrent*)current;
-(BOOL) supportsCsharpSerializable:(ICECurrent*)current;
-(BOOL) supportsNullOptional:(ICECurrent*)current;
@end

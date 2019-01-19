//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef INSTRUMENTATION_I_H
#define INSTRUMENTATION_I_H

void testRetryCount(int);
void testFailureCount(int);
void testInvocationCount(int);

void initCounts();
Ice::Instrumentation::CommunicatorObserverPtr getObserver();

#endif

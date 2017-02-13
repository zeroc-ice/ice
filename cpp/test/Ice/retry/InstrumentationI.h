// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef INSTRUMENTATION_I_H
#define INSTRUMENTATION_I_H

void testRetryCount(int);
void testFailureCount(int);
void testInvocationCount(int);

void initCounts();
Ice::Instrumentation::CommunicatorObserverPtr getObserver();

#endif

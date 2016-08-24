// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#pragma once

#if defined(__SLICE2JAVA__) && defined(__SLICE2JAVA_COMPAT__)
#   define JAVA_COMPAT 1
#endif 
  
//
// The following is needed only if this Slice file may be compiled with
// a slice2java compiler version < 3.7
//
// __SLICE2JAVA__ and __ICE_VERSION__ were both introduced in Ice 3.5.1
  
#if (defined(__SLICE2JAVA__) && (__ICE_VERSION__ < 30700)) || !defined(__ICE_VERSION__)
#   define JAVA_COMPAT 1
#endif

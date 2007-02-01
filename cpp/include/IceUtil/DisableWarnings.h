// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_UTIL_DISABLEWARNINGS_H
#define ICE_UTIL_DISABLEWARNINGS_H

//
// This header file disables various annoying compiler warnings that
// we don't want.
//
// IMPORTANT: Do *not* include this header file in another public header file!
//            Doing this may potentially disable the warnings in the source
//            code of our customers, which would be bad. Only include this
//            header file in Ice *source* files!
//

#if defined(_MSC_VER) && _MSC_VER >= 1400
#    define _CRT_SECURE_NO_DEPRECATE 1  // C4996 '<C function>' was declared deprecated/
#endif

#if defined(_MSC_VER) && _MSC_VER >= 1300
#    pragma warning( 4 : 4996 ) // C4996 'std::<function>' was declared deprecated
#endif

#endif

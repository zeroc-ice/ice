// **********************************************************************
//
// Copyright (c) 2002
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_CONFIG_H
#define ICE_CONFIG_H

#ifdef WIN32

// ... identifier was truncated to '255' characters in the debug information
#   pragma warning( disable : 4786 )
// 'this' : used in base member initializer list
#   pragma warning( disable : 4355 )

#ifdef ICE_EXPORTS
#   define ICE_API __declspec(dllexport)
#else
#   define ICE_API __declspec(dllimport)
#endif

#else // !WIN32

#   define ICE_API /**/

#endif

#endif

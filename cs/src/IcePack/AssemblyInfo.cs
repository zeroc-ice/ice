// **********************************************************************
//
// Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************


using System.Reflection;
using System.Runtime.CompilerServices;

[assembly: AssemblyTitle("icepackcs")]
[assembly: AssemblyDescription("IcePack run-time support")]
[assembly: AssemblyCompany("ZeroC, Inc.")]
[assembly: AssemblyConfiguration("")]
[assembly: AssemblyProduct("IcePack for C#")]
[assembly: AssemblyCopyright("Copyright (c) 2003-2004, ZeroC, Inc.")]
[assembly: AssemblyTrademark("Ice")]
[assembly: AssemblyCulture("")]		
[assembly: AssemblyVersion("2.1.0")]
[assembly: AssemblyDelaySign(false)]
#if __MonoCS__
[assembly: AssemblyKeyFile("IcePackcsKey.snk")] // mcs uses different search algorithm.
#else
[assembly: AssemblyKeyFile(@"..\..\IcePackcsKey.snk")]
#endif
[assembly: AssemblyKeyName("")]

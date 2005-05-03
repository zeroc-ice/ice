// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System.Reflection;
using System.Runtime.CompilerServices;

[assembly: AssemblyTitle("glacier2cs")]
[assembly: AssemblyDescription("Glacier2 run-time support")]
[assembly: AssemblyCompany("ZeroC, Inc.")]
[assembly: AssemblyConfiguration("")]
[assembly: AssemblyProduct("Glacier2 for C#")]
[assembly: AssemblyCopyright("Copyright (c) 2003-2005, ZeroC, Inc.")]
[assembly: AssemblyTrademark("Ice")]
[assembly: AssemblyCulture("")]		
[assembly: AssemblyVersion("2.2.0")]
[assembly: AssemblyDelaySign(false)]
#if __MonoCS__
[assembly: AssemblyKeyFile("Glacier2csKey.snk")] // mcs uses different search algorithm.
#else
[assembly: AssemblyKeyFile(@"..\..\Glacier2csKey.snk")]
#endif
[assembly: AssemblyKeyName("")]

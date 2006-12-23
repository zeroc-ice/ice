// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using System.Reflection;
using System.Runtime.CompilerServices;

[assembly: CLSCompliant(true)]

[assembly: AssemblyTitle("icestormcs")]
[assembly: AssemblyDescription("IceStorm run-time support")]
[assembly: AssemblyCompany("ZeroC, Inc.")]
[assembly: AssemblyConfiguration("")]
[assembly: AssemblyProduct("IceStorm for C#")]
[assembly: AssemblyCopyright("Copyright (c) 2003-2005, ZeroC, Inc.")]
[assembly: AssemblyTrademark("Ice")]
[assembly: AssemblyCulture("")]		
[assembly: AssemblyVersion("3.2.0")]
[assembly: AssemblyDelaySign(false)]
#if !__MonoCS__
#if MAKEFILE_BUILD
[assembly: AssemblyKeyFile("../../config/IcecsKey.snk")]
#else
[assembly: AssemblyKeyFile("../../../../config/IcecsKey.snk")]
#endif
#endif
[assembly: AssemblyKeyName("")]

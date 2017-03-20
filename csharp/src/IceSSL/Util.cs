// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

namespace IceSSL
{
    using System;
    using System.Diagnostics;
    using System.Security.Cryptography.X509Certificates;

    public sealed class Util
    {
        public static X509Certificate2 createCertificate(string certPEM)
        {
            char[] chars = certPEM.ToCharArray();
            byte[] bytes = new byte[chars.Length];
            for(int i = 0; i < chars.Length; ++i)
            {
                bytes[i] = (byte)chars[i];
            }
            return new X509Certificate2(bytes);
        }
    }
}

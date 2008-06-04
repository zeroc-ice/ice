<%@ WebHandler Language="c#" class="Test.Shutdown" %>

// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System.Web;

namespace Test
{
    public class Shutdown : IHttpHandler
    {
        public void ProcessRequest(HttpContext context)
        {
	    System.Environment.Exit(0);
        }

        public bool IsReusable
        {
            get
            {
                return true;
            }
        }
    }
}

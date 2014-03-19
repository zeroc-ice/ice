// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using System.Net;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Documents;
using System.Windows.Ink;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Animation;
using System.Windows.Shapes;
using Demo;

namespace bidir
{
    public class CallbackReceiverI : CallbackReceiverDisp_
    {
        public CallbackReceiverI(MainPage page)
        {
            _page = page;
        }

        public override void callback(int num, Ice.Current current)
        {
            _page.appendText("received callback #" + num.ToString() + Environment.NewLine);
        }

        private MainPage _page;
    }
}

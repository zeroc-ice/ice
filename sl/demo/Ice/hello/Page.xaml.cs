// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Documents;
using System.Windows.Ink;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Animation;
using System.Windows.Shapes;

using System.Net;
using System.IO;
using System.Collections;
using System.Text;
using System.Windows.Browser.Net;

namespace helloC
{
    public partial class Page : Canvas
    {
        private class AMI_Hello_sayHelloI : Demo.AMI_Hello_sayHello
        {
            public override void ice_response()
            {
                _tb.Text = "Call succeeded";
            }

            public override void ice_exception(Ice.Exception ex)
            {
                _tb.Text = "Call failed with exception:\n" + ex.ToString();
            }
        }

        public void Page_Loaded(object o, EventArgs e)
        {
            // Required to initialize variables
            InitializeComponent();
            Button1.MouseLeftButtonUp += new MouseEventHandler(OnClick);
            Button2.MouseLeftButtonUp += new MouseEventHandler(OnClickAMI);
            
            _tb = Button3.Children[0] as TextBlock;

            try
            {
                Ice.InitializationData initData = new Ice.InitializationData();
                initData.properties = Ice.Util.createProperties();
                initData.properties.setProperty("Ice.BridgeUri", "http://localhost:1287/IceBridge.ashx");
                Ice.Communicator comm = Ice.Util.initialize(initData);
                _hello = Demo.HelloPrxHelper.uncheckedCast(comm.stringToProxy("hello:tcp -p 10000"));
            }
            catch(Exception ex)
            {
                _tb.Text = "Initialization failed with exception:\n" + ex.ToString();
            }
        }
        public Page()
        {
            //this.Loaded += new EventHandler(EventHandlingCanvas_Loaded);
        }

        void EventHandlingCanvas_Loaded(object sender, EventArgs e)
        { 
        }

        void OnClick(object sender, MouseEventArgs e)
        {
            try
            {
                _hello.sayHello(0);
                _tb.Text = "Call succeeded";
            }
            catch (Exception ex)
            {
                _tb.Text = "Call failed with exception:\n" + ex.ToString();
            }
        }       

        void OnClickAMI(object sender, MouseEventArgs e)
        {
            try
            {
                _hello.sayHello_async(new AMI_Hello_sayHelloI(), 0);
                _tb.Text = "Calling sayHello()...";
            }
            catch (Exception ex)
            {
                _tb.Text = "Call failed with exception:\n" + ex.ToString();
                return;
            }
        }       

        private Demo.HelloPrx _hello;
        private static TextBlock _tb;
    }
}

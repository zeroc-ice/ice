// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
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
        public void Page_Loaded(object o, EventArgs e)
        {
            // Required to initialize variables
            InitializeComponent();
            Button1.MouseLeftButtonUp += new MouseEventHandler(OnClick);
            Button2.MouseLeftButtonUp += new MouseEventHandler(OnClickOneway);
            Button3.MouseLeftButtonUp += new MouseEventHandler(OnClickAMI);
            
            Button1.MouseLeftButtonDown += new MouseEventHandler(OnPress);
            Button2.MouseLeftButtonDown += new MouseEventHandler(OnPress);
            Button3.MouseLeftButtonDown += new MouseEventHandler(OnPress);

            _tb = Button4.Children[0] as TextBlock;

            try
            {
                Ice.InitializationData initData = new Ice.InitializationData();
                initData.properties = Ice.Util.createProperties();
                initData.properties.setProperty("Ice.BridgeUri", "http://localhost:1287/IceBridge.ashx");
                _comm = Ice.Util.initialize(initData);
                _hello = Demo.HelloPrxHelper.uncheckedCast(_comm.stringToProxy("hello:tcp -p 10000"));
                _helloOneway = Demo.HelloPrxHelper.uncheckedCast(_hello.ice_oneway());
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

        ~Page()
        {
            if(_comm != null)
            {
                try
                {
                    _comm.destroy();
                }
                catch(Exception ex)
                {
                    _tb.Text = "Destroy failed with exception:\n" + ex.ToString();
                }
            }
        }


        void EventHandlingCanvas_Loaded(object sender, EventArgs e)
        { 
        }

        void OnPress(object sender, MouseEventArgs e)
        {
            _tb.Text = "";
        }

        void OnClick(object sender, MouseEventArgs e)
        {
            //
            // Send regular twoway call. This will block the UI thread
            // until a response is returned from the server.
            //
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

        void OnClickOneway(object sender, MouseEventArgs e)
        {
            //
            // Send oneway call. This will return as soon as the request
            // as been sent and unblock the UI thread. However there is no
            // guarantee that the send has succeeded.
            //
            try
            {
                _helloOneway.sayHello(0);
                _tb.Text = "Call completed";
            }
            catch (Exception ex)
            {
                _tb.Text = "Call failed with exception:\n" + ex.ToString();
            }
        }       

        void sayHelloResponse()
        {
            _tb.Text = "Call succeeded";
        }

        void sayHelloException(Ice.Exception ex)
        {
            _tb.Text = "Call failed with exception:\n" + ex.ToString();
        }

        void OnClickAMI(object sender, MouseEventArgs e)
        {
            //
            // Send aynschronous call. This will return as soon as the request
            // as been sent and unblock the UI thread. A callback will then 
            // be called when it is known whether the call has succeeded or not.
            //
            try
            {
                _hello.sayHello_async(sayHelloResponse, sayHelloException, 0);
                _tb.Text = "Calling sayHello()...";
            }
            catch (Exception ex)
            {
                _tb.Text = "Call failed with exception:\n" + ex.ToString();
                return;
            }
        }       
        
        private Ice.Communicator _comm = null;
        private Demo.HelloPrx _hello;
        private Demo.HelloPrx _helloOneway;
        private static TextBlock _tb;
    }
}

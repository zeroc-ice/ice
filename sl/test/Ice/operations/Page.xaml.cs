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
using System.Collections.Generic;

namespace operationsC
{
    public partial class Page : Canvas
    {
        private static TextBlock _tb;
        public void Page_Loaded(object o, EventArgs e)
        {
            // Required to initialize variables
            InitializeComponent();
            Button1.MouseLeftButtonUp += new MouseEventHandler(OnClick);
            
            _tb = Button2.Children[0] as TextBlock;
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
                Ice.InitializationData initData = new Ice.InitializationData();
                initData.properties = Ice.Util.createProperties();
                initData.properties.setProperty("Ice.BridgeUri", "http://localhost:1287/IceBridge.ashx");

                //
                // In this test, we need at least two threads in the
                // client side thread pool for nested AMI.
                //
                initData.properties.setProperty("Ice.ThreadPool.Client.Size", "2");
                initData.properties.setProperty("Ice.ThreadPool.Client.SizeWarn", "0");

                //
                // We must set MessageSizeMax to an explicit values,
                // because we run tests to check whether
                // Ice.MemoryLimitException is raised as expected.
                //
                initData.properties.setProperty("Ice.MessageSizeMax", "100");

                _comm = Ice.Util.initialize(initData);

                _myClass = AllTests.allTests(_comm, out _callbacks);
            }
            catch (Exception ex)
            {
                _tb.Text = ex.ToString();
                return;
            }
            _tb.Text = "Waiting for AMI Callbacks";
            (Resources.FindName("timer") as Storyboard).Begin();
        }       

        void CheckCallbacks(object sender, EventArgs e)
        {
            List<Callback> active = new List<Callback>();
            foreach(Callback c in _callbacks)
            {
                if(!c.check())
                {
                    active.Add(c);
                }
            }
            _callbacks = active;

            _tb.Text = "Callbacks = " + _callbacks.Count;

            if(_callbacks.Count == 0)
            {
                _myClass.shutdown();
                _tb.Text = "Test Passed";

                try
                {
                    _comm.destroy();
                }
                catch (Exception ex)
                {
                    _tb.Text = ex.ToString();
                }
            }
            else
            {
                (Resources.FindName("timer") as Storyboard).Begin();
            }
        }

        private Ice.Communicator _comm;
        private List<Callback> _callbacks;
        private Test.MyClassPrx _myClass;
    }
}

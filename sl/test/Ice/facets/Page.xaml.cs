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

namespace facetsC
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
            Ice.Communicator comm = null;
            try
            {
                Ice.InitializationData initData = new Ice.InitializationData();
                initData.properties = Ice.Util.createProperties();
                initData.properties.setProperty("Ice.BridgeUri", "http://localhost:1287/IceBridge.ashx");
                comm = Ice.Util.initialize(initData);

                Test.GPrx g = AllTests.allTests(comm);
                g.shutdown();
            }
            catch (Exception ex)
            {
                _tb.Text = ex.ToString();
                return;
            }
            _tb.Text = "Test Passed";

            if(comm != null)
            {
                try
                {
                    comm.destroy();
                }
                catch (Exception ex)
                {
                    _tb.Text = ex.ToString();
                }
            }
        }       
    }
}

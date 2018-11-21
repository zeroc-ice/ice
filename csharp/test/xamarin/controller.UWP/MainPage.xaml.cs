// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

namespace controller.UWP
{
    public sealed partial class MainPage : Test.PlatformAdapter
    {
        public MainPage()
        {
            InitializeComponent();
            LoadApplication(new controller.App(this));
        }

        public bool isEmulator()
        {
            return false;
        }

        public string processControllerIdentity()
        {
            return "UWP/ProcessController";
        }

        public string processControllerRegistryHost()
        {
            return "127.0.0.1";
        }
    }
}

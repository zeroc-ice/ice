// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
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
            return "UWPXamarin/ProcessController";
        }

        public string processControllerRegistryHost()
        {
            return "127.0.0.1";
        }
    }
}

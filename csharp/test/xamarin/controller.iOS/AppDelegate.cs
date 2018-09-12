// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using Foundation;
using UIKit;

namespace controller.iOS
{
    // The UIApplicationDelegate for the application. This class is responsible for launching the
    // User Interface of the application, as well as listening (and optionally responding) to
    // application events from iOS.
    [Register("AppDelegate")]
    public partial class AppDelegate : global::Xamarin.Forms.Platform.iOS.FormsApplicationDelegate,
                                       Test.PlatformAdapter
    {
        public string processControllerIdentity()
        {
            if(isEmulator())
            {
                return "iPhoneSimulator/com.zeroc.Xamarin-Test-Controller";
            }
            else
            {
                return "iPhoneOS/com.zeroc.Xamarin-Test-Controller";
            }
        }

        //
        // This method is invoked when the application has loaded and is ready to run. In this
        // method you should instantiate the window, load the UI into it and then make the window
        // visible.
        //
        // You have 17 seconds to return from this method, or iOS will terminate your application.
        //
        public override bool FinishedLaunching(UIApplication app, NSDictionary options)
        {
            global::Xamarin.Forms.Forms.Init();
            LoadApplication(new App(this));

            return base.FinishedLaunching(app, options);
        }

        public bool isEmulator()
        {
            return true;
        }

        public string processControllerRegistryHost()
        {
            return "127.0.0.1";
        }

        public bool registerProcessController()
        {
            return true;
        }
    }
}

//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

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
            if (isEmulator())
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
            return ObjCRuntime.Runtime.Arch == ObjCRuntime.Arch.SIMULATOR;
        }

        public string processControllerRegistryHost()
        {
            return isEmulator() ? "127.0.0.1" : ""; // With an empty host, the controller will use IceDiscovery.
        }
    }
}

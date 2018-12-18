// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

using System;
using Android.App;
using Android.Content.PM;
using Android.OS;
using Android.Util;

namespace controller.Droid
{
    public class AndroidLogger : Ice.Logger
    {
        private const string TAG = "ControllerApp";
        private string _prefix;

        public AndroidLogger(string prefix)
        {
            _prefix = prefix;
        }

        public void print(string message)
        {
            Log.Debug(TAG, message);
        }

        public void trace(String category, String message)
        {
            Log.Verbose(category, message);
        }

        public void warning(String message)
        {
            Log.Warn(TAG, message);
        }

        public void error(String message)
        {
            Log.Error(TAG, message);
        }

        public String getPrefix()
        {
            return _prefix;
        }

        public Ice.Logger cloneWithPrefix(string s)
        {
            return new AndroidLogger(s);
        }
    }

    [Activity(Label = "controller",
              Name = "controller.MainActivity",
              Icon = "@mipmap/icon",
              Theme = "@style/MainTheme",
              MainLauncher = true,
              ConfigurationChanges = ConfigChanges.ScreenSize | ConfigChanges.Orientation)]
    public class MainActivity : global::Xamarin.Forms.Platform.Android.FormsAppCompatActivity,
                                Test.PlatformAdapter
    {
        protected override void OnCreate(Bundle bundle)
        {
            Ice.Util.setProcessLogger(new AndroidLogger(""));
            TabLayoutResource = Resource.Layout.Tabbar;
            ToolbarResource = Resource.Layout.Toolbar;

            base.OnCreate(bundle);

            global::Xamarin.Forms.Forms.Init(this, bundle);
            LoadApplication(new App(this));
        }

        public bool isEmulator()
        {
            return Build.Fingerprint.Contains("vbox") ||
                   Build.Fingerprint.Contains("generic") ||
                   Build.Fingerprint.Contains("unknown") ||
                   Build.Model.Contains("google_sdk") ||
                   Build.Model.Contains("Emulator") ||
                   Build.Model.Contains("Android SDK built for x86") ||
                   Build.Manufacturer.Contains("Genymotion") ||
                   (Build.Brand.StartsWith("generic") && Build.Device.StartsWith("generic")) ||
                   Build.Product.Equals("google_sdk");
        }

        public string processControllerIdentity()
        {
            return "Android/ProcessController";
        }

        public string processControllerRegistryHost()
        {
            return isEmulator() ? "10.0.2.2" : ""; // With an empty host, the controller will use IceDiscovery.
        }
    }
}

// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

using System;
using System.Text;
using System.IO;
using Xamarin.Forms;
using System.Net;
using System.Net.Sockets;
using System.Net.NetworkInformation;
using System.Threading;
using System.Collections.ObjectModel;
using System.Linq;
using System.Reflection;
using System.Threading.Tasks;
using Test;
using Ice;
using Test.Common;
using System.Security.Cryptography.X509Certificates;

namespace controller
{
    public class TestFactory
    {
        public static TestHelper create(string type)
        {
            if(type.Equals("Ice.acm.Server"))
            {
                return new Ice.acm.Server();
            }
            else if(type.Equals("Ice.acm.Client"))
            {
                return new Ice.acm.Client();
            }

            else if(type.Equals("Ice.adapterDeactivation.Server"))
            {
                return new Ice.adapterDeactivation.Server();
            }
            else if(type.Equals("Ice.adapterDeactivation.Client"))
            {
                return new Ice.adapterDeactivation.Client();
            }
            else if(type.Equals("Ice.adapterDeactivation.Collocated"))
            {
                return new Ice.adapterDeactivation.Collocated();
            }

            else if(type.Equals("Ice.admin.Server"))
            {
                return new Ice.admin.Server();
            }
            else if(type.Equals("Ice.admin.Client"))
            {
                return new Ice.admin.Client();
            }

            else if(type.Equals("Ice.ami.Server"))
            {
                return new Ice.ami.Server();
            }
            else if(type.Equals("Ice.ami.Client"))
            {
                return new Ice.ami.Client();
            }
            else if(type.Equals("Ice.ami.Collocated"))
            {
                return new Ice.ami.Collocated();
            }

            else if(type.Equals("Ice.binding.Server"))
            {
                return new Ice.binding.Server();
            }
            else if(type.Equals("Ice.binding.Client"))
            {
                return new Ice.binding.Client();
            }

            else if(type.Equals("Ice.checksum.Server"))
            {
                return new Ice.checksum.Server();
            }
            else if(type.Equals("Ice.checksum.Client"))
            {
                return new Ice.checksum.Client();
            }

            else if(type.Equals("Ice.defaultServant.Client"))
            {
                return new Ice.defaultServant.Client();
            }

            else if(type.Equals("Ice.defaultValue.Client"))
            {
                return new Ice.defaultValue.Client();
            }

            else if(type.Equals("Ice.dictMapping.Client"))
            {
                return new Ice.dictMapping.Client();
            }
            else if(type.Equals("Ice.dictMapping.Collocated"))
            {
                return new Ice.dictMapping.Collocated();
            }
            else if(type.Equals("Ice.dictMapping.Server"))
            {
                return new Ice.dictMapping.Server();
            }
            else if(type.Equals("Ice.dictMapping.Serveramd"))
            {
                return new Ice.dictMapping.AMD.Server();
            }

            else if(type.Equals("Ice.enums.Client"))
            {
                return new Ice.enums.Client();
            }
            else if(type.Equals("Ice.enums.Server"))
            {
                return new Ice.enums.Server();
            }

            else if(type.Equals("Ice.exceptions.Client"))
            {
                return new Ice.exceptions.Client();
            }
            else if(type.Equals("Ice.exceptions.Collocated"))
            {
                return new Ice.exceptions.Collocated();
            }
            else if(type.Equals("Ice.exceptions.Server"))
            {
                return new Ice.exceptions.Server();
            }
            else if(type.Equals("Ice.exceptions.Serveramd"))
            {
                return new Ice.exceptions.AMD.Server();
            }

            else if(type.Equals("Ice.facets.Client"))
            {
                return new Ice.facets.Client();
            }
            else if(type.Equals("Ice.facets.Collocated"))
            {
                return new Ice.facets.Collocated();
            }
            else if(type.Equals("Ice.facets.Server"))
            {
                return new Ice.facets.Server();
            }

            else if(type.Equals("Ice.hold.Client"))
            {
                return new Ice.hold.Client();
            }
            else if(type.Equals("Ice.hold.Server"))
            {
                return new Ice.hold.Server();
            }

            else if(type.Equals("Ice.info.Client"))
            {
                return new Ice.info.Client();
            }
            else if(type.Equals("Ice.info.Server"))
            {
                return new Ice.info.Server();
            }

            else if(type.Equals("Ice.inheritance.Client"))
            {
                return new Ice.inheritance.Client();
            }
            else if(type.Equals("Ice.inheritance.Collocated"))
            {
                return new Ice.inheritance.Collocated();
            }
            else if(type.Equals("Ice.inheritance.Server"))
            {
                return new Ice.inheritance.Server();
            }

            else if(type.Equals("Ice.interceptor.Client"))
            {
                return new Ice.interceptor.Client();
            }

            else if(type.Equals("Ice.invoke.Client"))
            {
                return new Ice.invoke.Client();
            }
            else if(type.Equals("Ice.invoke.Server"))
            {
                return new Ice.invoke.Server();
            }

            else if(type.Equals("Ice.location.Client"))
            {
                return new Ice.location.Client();
            }
            else if(type.Equals("Ice.location.Server"))
            {
                return new Ice.location.Server();
            }

            else if(type.Equals("Ice.objects.Client"))
            {
                return new Ice.objects.Client();
            }
            else if(type.Equals("Ice.objects.Collocated"))
            {
                return new Ice.objects.Collocated();
            }
            else if(type.Equals("Ice.objects.Server"))
            {
                return new Ice.objects.Server();
            }

            else if(type.Equals("Ice.operations.Client"))
            {
                return new Ice.operations.Client();
            }
            else if(type.Equals("Ice.operations.Collocated"))
            {
                return new Ice.operations.Collocated();
            }
            else if(type.Equals("Ice.operations.Server"))
            {
                return new Ice.operations.Server();
            }
            else if(type.Equals("Ice.operations.Serveramd"))
            {
                return new Ice.operations.AMD.Server();
            }
            else if(type.Equals("Ice.operations.Serveramdtie"))
            {
                return new Ice.operations.AMD.tie.Server();
            }
            else if(type.Equals("Ice.operations.Servertie"))
            {
                return new Ice.operations.tie.Server();
            }

            else if(type.Equals("Ice.optional.Client"))
            {
                return new Ice.optional.Client();
            }
            else if(type.Equals("Ice.optional.Server"))
            {
                return new Ice.optional.Server();
            }
            else if(type.Equals("Ice.optional.Serveramd"))
            {
                return new Ice.optional.AMD.Server();
            }

            else if(type.Equals("Ice.namespacemd.Client"))
            {
                return new Ice.namespacemd.Client();
            }
            else if(type.Equals("Ice.namespacemd.Server"))
            {
                return new Ice.namespacemd.Server();
            }

            else if(type.Equals("Ice.proxy.Client"))
            {
                return new Ice.proxy.Client();
            }
            else if(type.Equals("Ice.proxy.Collocated"))
            {
                return new Ice.proxy.Collocated();
            }
            else if(type.Equals("Ice.proxy.Server"))
            {
                return new Ice.proxy.Server();
            }
            else if(type.Equals("Ice.proxy.Serveramd"))
            {
                return new Ice.proxy.AMD.Server();
            }

            else if(type.Equals("Ice.retry.Client"))
            {
                return new Ice.retry.Client();
            }
            else if(type.Equals("Ice.retry.Collocated"))
            {
                return new Ice.retry.Collocated();
            }
            else if(type.Equals("Ice.retry.Server"))
            {
                return new Ice.retry.Server();
            }

            else if(type.Equals("Ice.scope.Client"))
            {
                return new Ice.scope.Client();
            }
            else if(type.Equals("Ice.scope.Server"))
            {
                return new Ice.scope.Server();
            }

            else if(type.Equals("Ice.seqMapping.Client"))
            {
                return new Ice.seqMapping.Client();
            }
            else if(type.Equals("Ice.seqMapping.Collocated"))
            {
                return new Ice.seqMapping.Collocated();
            }
            else if(type.Equals("Ice.seqMapping.Server"))
            {
                return new Ice.seqMapping.Server();
            }
            else if(type.Equals("Ice.seqMapping.Serveramd"))
            {
                return new Ice.seqMapping.AMD.Server();
            }

            else if(type.Equals("Ice.serialize.Client"))
            {
                return new Ice.serialize.Client();
            }

            else if(type.Equals("Ice.servantLocator.Client"))
            {
                return new Ice.servantLocator.Client();
            }
            else if(type.Equals("Ice.servantLocator.Collocated"))
            {
                return new Ice.servantLocator.Collocated();
            }
            else if(type.Equals("Ice.servantLocator.Server"))
            {
                return new Ice.servantLocator.Server();
            }
            else if(type.Equals("Ice.servantLocator.Serveramd"))
            {
                return new Ice.servantLocator.AMD.Server();
            }

            else if(type.Equals("Ice.stream.Client"))
            {
                return new Ice.stream.Client();
            }

            else if(type.Equals("Ice.threadPoolPriority.Client"))
            {
                return new Ice.threadPoolPriority.Client();
            }
            else if(type.Equals("Ice.threadPoolPriority.Server"))
            {
                return new Ice.threadPoolPriority.Server();
            }

            else if(type.Equals("Ice.timeout.Client"))
            {
                return new Ice.timeout.Client();
            }
            else if(type.Equals("Ice.timeout.Server"))
            {
                return new Ice.timeout.Server();
            }

            else if(type.Equals("Ice.udp.Client"))
            {
                return new Ice.udp.Client();
            }
            else if(type.Equals("Ice.udp.Server"))
            {
                return new Ice.udp.Server();
            }

            else
            {
                return null;
            }
        }
    }

    public class ControllerHelperI : ControllerHelper
    {
        public ControllerHelperI(string name, string[] args, MainPage page, PlatformAdapter platformAdapter)
        {
            _name = name;
            _args = args;
            _page = page;
            _platformAdapter = platformAdapter;
            _typename = name;
        }

        public void communicatorInitialized(Communicator communicator)
        {
            var properties = communicator.getProperties();
            if(properties.getProperty("Ice.Plugin.IceSSL").Equals("IceSSL.dll:IceSSL.PluginFactory"))
            {
                var plugin =(IceSSL.Plugin)communicator.getPluginManager().getPlugin("IceSSL");
                plugin.setCertificates(loadCertificate(properties.getProperty("IceSSL.CertFile"),
                                                       properties.getProperty("IceSSL.Password")));
                plugin.setCACertificates(loadCertificate(properties.getProperty("IceSSL.CAs")));
                communicator.getPluginManager().initializePlugins();

                if(communicator.getProperties().getPropertyWithDefault("Ice.Admin.DelayCreation", "0").Equals("1"))
                {
                    communicator.getAdmin();
                }
            }
        }

        public X509Certificate2Collection loadCertificate(string name, string password = "")
        {
            var assembly = IntrospectionExtensions.GetTypeInfo(typeof(MainPage)).Assembly;
            var stream = assembly.GetManifestResourceStream(string.Format("controller.{0}", name));
            var memoryStream = new MemoryStream();
            stream.CopyTo(memoryStream);
            var cert = new X509Certificate2Collection();
            if(string.IsNullOrEmpty(password))
            {
                cert.Import(memoryStream.ToArray());
            }
            else
            {
                cert.Import(memoryStream.ToArray(), password, X509KeyStorageFlags.UserKeySet);
            }
            return cert;
        }

        public string getOutput()
        {
            return _helper.getWriter().ToString();
        }

        public void join()
        {
            if(_thread != null)
            {
                _thread.Join();
            }
        }

        public void completed(int status)
        {
            lock(this)
            {
                _completed = true;
                _status = status;
                Monitor.PulseAll(this);
            }
        }

        public void run()
        {
            _thread = new Thread(() =>
            {
                try
                {
                    _helper = TestFactory.create(_typename);
                    _helper.setControllerHelper(this);
                    _helper.setWriter(new StringWriter());
                    _helper.run(_args);
                    completed(0);
                }
                catch(System.Exception ex)
                {
                    _helper.getWriter().WriteLine("unexpected unknown exception while running `{0}':\n", ex);
                    completed(1);
                }
            });
            _thread.Start();
        }

        public string loggerPrefix()
        {
            return _name;
        }

        public void serverReady()
        {
            lock(this)
            {
                _ready = true;
                Monitor.PulseAll(this);
            }
        }

        public void shutdown()
        {
            lock(this)
            {
                if(_completed)
                {
                    return;
                }

                if(_helper != null)
                {
                    var communicator = _helper.communicator();
                    if(communicator != null)
                    {
                        communicator.shutdown();
                    }
                }
            }
        }

        public void waitReady(int msec)
        {
            lock(this)
            {
                while(!_ready && !_completed)
                {
                    if(!Monitor.Wait(this, msec * 1000))
                    {
                        throw new ProcessFailedException("timed out waiting for the process to be ready");
                    }
                }
            }
        }

        public int waitSuccess(int msec)
        {
            lock(this)
            {
                while(!_completed)
                {
                    if(!Monitor.Wait(this, msec * 1000))
                    {
                        throw new ProcessFailedException("timed out waiting for the process to be ready");
                    }
                }
                return _status;
            }
        }

        private MainPage _page;
        private PlatformAdapter _platformAdapter;
        private string _name;
        private string[] _args;
        private TestHelper _helper;
        private bool _ready;
        private bool _completed;
        private int _status;
        private Thread _thread;
        private string _typename;
    }

    public class ProccessI : Test.Common.ProcessDisp_
    {
        public ProccessI(ControllerHelperI controllerHelper)
        {
            _controllerHelper = controllerHelper;
        }

        public override void waitReady(int timeout, Current current)
        {
            _controllerHelper.waitReady(timeout);
        }

        public override int waitSuccess(int timeout, Current current)
        {
            return _controllerHelper.waitSuccess(timeout);
        }

        public override string terminate(Current current)
        {
            _controllerHelper.shutdown();
            current.adapter.remove(current.id);
            _controllerHelper.join();
            return _controllerHelper.getOutput();
        }

        private ControllerHelperI _controllerHelper;
    }

    public class ProcessControllerI : Test.Common.ProcessControllerDisp_
    {
        public ProcessControllerI(MainPage mainPage)
        {
            _mainPage = mainPage;
        }

        public override Test.Common.ProcessPrx start(string testsuite, string exe, string[] args, Current current)
        {
            var test = testsuite.Replace("/", ".") + "." + char.ToUpper(exe[0]) + exe.Substring(1);
            _mainPage.print("starting test." + test + "... ");
            string[] newArgs = new string[args.Length + 1];
            newArgs[0] = test;
            Array.Copy(args, 0, newArgs, 1, args.Length);
            var helper = new ControllerHelperI(test, newArgs, _mainPage, _mainPage.platformAdapter);
            helper.run();
            return Test.Common.ProcessPrxHelper.uncheckedCast(current.adapter.addWithUUID(new ProccessI(helper)));
        }

        public override string getHost(string protocol, bool ipv6, Current current)
        {
            if(_mainPage.platformAdapter.isEmulator())
            {
                return "127.0.0.1";
            }
            else
            {
                return _mainPage.getHost(ipv6);
            }
        }

        private MainPage _mainPage;
    }

    public class ControllerI
    {
        public ControllerI(MainPage mainPage)
        {
            IceSSL.Util.registerIceSSL(false);
            IceDiscovery.Util.registerIceDiscovery(false);
            _mainPage = mainPage;
            var initData = new InitializationData();
            initData.properties = Util.createProperties();
            initData.properties.setProperty("Ice.ThreadPool.Client.SizeMax", "10");
            initData.properties.setProperty("Ice.ThreadPool.Server.SizeMax", "10");
            initData.properties.setProperty("ControllerAdapter.Endpoints", "tcp");
            initData.properties.setProperty("Ice.Override.ConnectTimeout", "1000");
            //initData.properties.setProperty("Ice.Trace.Network", "3");
            //initData.properties.setProperty("Ice.Trace.Protocol", "1");
            initData.properties.setProperty("ControllerAdapter.AdapterId", new Guid().ToString());

            if(!mainPage.platformAdapter.isEmulator())
            {
                initData.properties.setProperty("Ice.Plugin.IceDiscovery", "IceDiscovery:IceDiscovery.PluginFactory");
                initData.properties.setProperty("IceDiscovery.DomainId", "TestController");
            }

            _communicator = Util.initialize(initData);

            _adapter = _communicator.createObjectAdapter("ControllerAdapter");
            _processController = ProcessControllerPrxHelper.uncheckedCast(
                _adapter.add(new ProcessControllerI(mainPage),
                             Util.stringToIdentity(mainPage.platformAdapter.processControllerIdentity())));
            _adapter.activate();

            registerProcessController();
            _mainPage.print(mainPage.platformAdapter.processControllerIdentity());
        }

        public void
        registerProcessController()
        {
            Task.Run(async() =>
                {
                    try
                    {
                        if(_mainPage.RegisterProcessControllerEnabled())
                        {
                            ProcessControllerRegistryPrx registry;
                            if(_mainPage.processControllerRegistryHost().Length == 0)
                            {
                                registry = ProcessControllerRegistryPrxHelper.uncheckedCast(
                                    _communicator.stringToProxy("Util/ProcessControllerRegistry"));
                            }
                            else
                            {
                                registry = ProcessControllerRegistryPrxHelper.uncheckedCast(
                                    _communicator.stringToProxy(string.Format(
                                        "Util/ProcessControllerRegistry:tcp -h {0} -p 15001",
                                        _mainPage.processControllerRegistryHost())));
                            }
                            await registry.ice_pingAsync();
                            var connection = registry.ice_getConnection();
                            connection.setAdapter(_adapter);
                            connection.setACM(5, ACMClose.CloseOff, ACMHeartbeat.HeartbeatAlways);
                            connection.setCloseCallback(conn =>
                                {
                                    _mainPage.print("connection with process controller registry closed");
                                    Thread.Sleep(500);
                                    registerProcessController();
                                });
                            await registry.setProcessControllerAsync(_processController);
                        }
                    }
                    catch(ConnectFailedException)
                    {
                        Thread.Sleep(500);
                        registerProcessController();
                    }
                    catch(Ice.TimeoutException)
                    {
                        Thread.Sleep(500);
                        registerProcessController();
                    }
                    catch(DNSException)
                    {
                        Thread.Sleep(500);
                        registerProcessController();
                    }
                    catch(System.Exception ex)
                    {
                        _mainPage.print(ex.ToString());
                    }
                });
        }

        private Communicator _communicator;
        private MainPage _mainPage;
        private ObjectAdapter _adapter;
        private ProcessControllerPrx _processController;
    }

    public partial class MainPage : ContentPage
    {
        public MainPage(PlatformAdapter platformAdapter)
        {
            InitializeComponent();
            this.platformAdapter = platformAdapter;

            IPv4Address.Items.Add(IPAddress.Loopback.ToString());
            IPv4Address.SelectedIndex = 0;

            IPv6Address.Items.Add(IPAddress.IPv6Loopback.ToString());
            IPv6Address.SelectedIndex = 0;

            ProcessControllerRegistryHost.Text = platformAdapter.processControllerRegistryHost();

            ProcessControllerRegistryHost.TextChanged += ProcessControllerRegistryHost_TextChanged;
            RegisterProcessController.Clicked += RegisterProcessController_Clicked;

            NetworkInterface[] nics = NetworkInterface.GetAllNetworkInterfaces();
            foreach(NetworkInterface ni in nics)
            {
                IPInterfaceProperties ipProps = ni.GetIPProperties();
                UnicastIPAddressInformationCollection uniColl = ipProps.UnicastAddresses;
                foreach(UnicastIPAddressInformation uni in uniColl)
                {
                    if(uni.Address.AddressFamily == AddressFamily.InterNetwork)
                    {
                        if(!uni.Address.Equals(IPAddress.Loopback))
                        {
                            IPv4Address.Items.Add(uni.Address.ToString());
                        }
                    }
                    else if(uni.Address.AddressFamily == AddressFamily.InterNetworkV6)
                    {
                        if(!uni.Address.Equals(IPAddress.IPv6Loopback))
                        {
                            IPv6Address.Items.Add(uni.Address.ToString());
                        }
                    }
                }
            }
            Output.ItemsSource = _outputSource;
            try
            {
                _controllerI = new ControllerI(this);
            }
            catch(PluginInitializationException ex)
            {
                print(ex.ToString());
                print(ex.reason);
                print(ex.StackTrace);
            }
            catch(System.Exception ex)
            {
                print(ex.ToString());
            }
        }

        public bool RegisterProcessControllerEnabled()
        {
            return !RegisterProcessController.IsEnabled;
        }

        private void RegisterProcessController_Clicked(object sender, EventArgs e)
        {
            RegisterProcessController.IsEnabled = false;
            _controllerI.registerProcessController();
        }

        private void ProcessControllerRegistryHost_TextChanged(object sender, TextChangedEventArgs e)
        {
            RegisterProcessController.IsEnabled = true;
        }

        public void print(string message)
        {
            Device.BeginInvokeOnMainThread(() =>
                {
                    _outputSource.Add(message);
                    Output.ScrollTo(Output.ItemsSource.Cast<object>().LastOrDefault(), ScrollToPosition.End, true);
                });
        }

        public string processControllerRegistryHost()
        {
            return ProcessControllerRegistryHost.Text;
        }

        public PlatformAdapter platformAdapter
        {
            get;
            private set;
        }

        public string getHost(bool ipv6)
        {
            return ipv6 ? IPv6Address.SelectedItem.ToString() : IPv4Address.SelectedItem.ToString();
        }

        private ObservableCollection<string> _outputSource = new ObservableCollection<string>();
        private ControllerI _controllerI;
    }
}

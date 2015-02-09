// **********************************************************************
//
// Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include "pch.h"
#include "MainPage.xaml.h"
#include <iostream>

using namespace std;
using namespace hello;
using namespace Demo;
using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::UI::Core;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Controls::Primitives;
using namespace Windows::UI::Xaml::Data;
using namespace Windows::UI::Xaml::Input;
using namespace Windows::UI::Xaml::Media;
using namespace Windows::UI::Xaml::Navigation;

MainPage::MainPage()
{
    InitializeComponent();
    mode->SelectedIndex = 0;
    Ice::InitializationData id;
	id.properties = Ice::createProperties();
	id.properties->setProperty("Ice.Plugin.IceDiscovery", "1"); // Enable the IceDiscovery plugin
    id.dispatcher = Ice::newDispatcher(
        [=](const Ice::DispatcherCallPtr& call, const Ice::ConnectionPtr&)
            {
                this->Dispatcher->RunAsync(
                    CoreDispatcherPriority::Normal, ref new DispatchedHandler([=]()
                        {
                            call->run();
                        }, CallbackContext::Any));
            });
    _communicator = Ice::initialize(id);
    updateProxy();
}

void
hello::MainPage::updateProxy()
{
    if (!_communicator)
    {
        return;
    }

    string h = IceUtil::wstringToString(hostname->Text->Data());
    if (h.empty() && !useDiscovery->IsChecked->Value)
    {
        print("Host is empty.");
        _helloPrx = 0;
        return;
    }
	Ice::ObjectPrx prx;
	if (useDiscovery->IsChecked->Value)
	{
		prx = _communicator->stringToProxy("hello");
	}
	else
	{
		prx = _communicator->stringToProxy("hello:tcp -h " + h + " -p 10000:ssl -h " + h +
			" -p 10001:udp -h " + h + " -p 10000");
	}

    switch(mode->SelectedIndex)
    {
        case 0:
        {
            prx = prx->ice_twoway();
            break;
        }
        case 1:
        {
            prx = prx->ice_twoway()->ice_secure(true);
            break;
        }
        case 2:
        {
            prx = prx->ice_oneway();
            break;
        }
        case 3:
        {
            prx = prx->ice_batchOneway();
            break;
        }
        case 4:
        {
            prx = prx->ice_oneway()->ice_secure(true);
            break;
        }
        case 5:
        {
            prx = prx->ice_batchOneway()->ice_secure(true);
            break;
        }
        case 6:
        {
            prx = prx->ice_datagram();
            break;
        }
        case 7:
        {
            prx = prx->ice_batchDatagram();
            break;
        }
        default:
        {
            break;
        }
    }

    if(timeout->Value > 0)
    {
        prx = prx->ice_invocationTimeout(static_cast<int>(timeout->Value * 1000));
    }
    _helloPrx = Demo::HelloPrx::uncheckedCast(prx);
    print("Ready.");
}

bool
hello::MainPage::isBatch()
{
    return mode->SelectedIndex == 3 || mode->SelectedIndex == 5 || mode->SelectedIndex == 7;
}

void
hello::MainPage::hello_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
    try
    {
        if(!_helloPrx)
        {
            return;
        }

        if(!isBatch())
        {
            print("Sending sayHello request.");
            _response = false;
            hello->IsEnabled = false;
            int deliveryMode = mode->SelectedIndex;
            Ice::AsyncResultPtr result = _helloPrx->begin_sayHello(static_cast<int>(delay->Value * 1000),
                                        [=]()
                                            {
                                                hello->IsEnabled = true;
                                                this->_response = true;
                                                print("Ready.");
                                            },
                                        [=](const Ice::Exception& ex)
                                            {
                                                hello->IsEnabled = true;
                                                this->_response = true;
                                                ostringstream os;
                                                os << ex;
                                                print(os.str());
                                            },
                                        [=](bool sentSynchronously)
                                            {
                                                if(_helloPrx)
                                                {
                                                    Ice::ConnectionPtr con = _helloPrx->ice_getCachedConnection();
                                                    if(con)
                                                    {
                                                        Ice::IPConnectionInfoPtr info = 
                                                            Ice::IPConnectionInfoPtr::dynamicCast(con->getInfo());
                                                        if(info)
                                                        {
                                                            hostname->Text = ref new String(
                                                                IceUtil::stringToWstring(info->remoteAddress).c_str());
                                                        }
                                                    }
                                                }
                                                if(this->_response)
                                                {
                                                    return; // Response was received already.
                                                }
                                                if(deliveryMode <= 1)
                                                {
                                                    print("Waiting for response.");
                                                }
                                                else if(!sentSynchronously)
                                                {
                                                    hello->IsEnabled = true;
                                                    print("Ready.");
                                                }
                                            });
            
            if(!result->sentSynchronously())
            {
                print("Sending request");
            }
            else if(deliveryMode > 1)
            {
                hello->IsEnabled = true;
                print("Ready.");
            }
        }
        else
        {
            print("Queued hello request.");
            _helloPrx->sayHello(static_cast<int>(delay->Value * 1000));
            flush->IsEnabled = true;
        }
    }
    catch(const Ice::Exception& ex)
    {
        ostringstream os;
        os << ex;
        print(os.str());
    }
}

void hello::MainPage::shutdown_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
    if(!_helloPrx)
    {
        return;
    }

    try
    {
        if(_helloPrx->ice_isBatchOneway() || _helloPrx->ice_isBatchDatagram())
        {
            print("Queued shutdown request.");
            _helloPrx->shutdown();
            flush->IsEnabled = true;
        }
        else
        {
            print("Shutting down...");
            shutdown->IsEnabled = false;
            _helloPrx->begin_shutdown([=]()
                                      {
                                          shutdown->IsEnabled = true;
                                          print("Ready.");
                                      },
                                      [=](const Ice::Exception& ex)
                                      {
                                          shutdown->IsEnabled = true;
                                          ostringstream os;
                                          os << ex;
                                          print(os.str());
                                      });
        }
    }
    catch(const Ice::Exception& ex)
    {
        ostringstream os;
        os << ex;
        print(os.str());
    }
}

void hello::MainPage::flush_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
    try
    {
        flush->IsEnabled = false;
        _communicator->begin_flushBatchRequests([=](const Ice::Exception& ex)
                                                    {
                                                        ostringstream os;
                                                        os << ex;
                                                        print(os.str());
                                                    },
                                                [=](bool)
                                                    {
                                                        print("Flushed batch requests.");
                                                    });
    }
    catch(const Ice::Exception& ex)
    {
        ostringstream os;
        os << ex;
        print(os.str());
    }
}

void 
MainPage::mode_SelectionChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::SelectionChangedEventArgs^ e)
{
    updateProxy();
}

void 
MainPage::timeout_ValueChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::Primitives::RangeBaseValueChangedEventArgs^ e)
{
    updateProxy();
}

void 
MainPage::hostname_TextChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::TextChangedEventArgs^ e)
{
    if (hostname->Text->Length() == 0 && !useDiscovery->IsChecked->Value)
    {
        hello->IsEnabled = false;
        shutdown->IsEnabled = false;
        flush->IsEnabled = false;
    }
    else
    {
        hello->IsEnabled = true;
        shutdown->IsEnabled = true;
        flush->IsEnabled = false;
    }
    updateProxy();
}

void hello::MainPage::useDiscovery_Changed(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	if (useDiscovery->IsChecked->Value)
	{
		hostname->Text = "";
		hostname->IsEnabled = false;
	}
	else
	{
		hostname->Text = "127.0.0.1";
		hostname->IsEnabled = true;
	}
	updateProxy();
}

void
MainPage::print(const std::string& message)
{
    output->Text = ref new String(IceUtil::stringToWstring(message).c_str());
}



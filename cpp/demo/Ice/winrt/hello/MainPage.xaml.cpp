// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
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
}

Demo::HelloPrx
hello::MainPage::proxy()
{
    string h = IceUtil::wstringToString(hostname->Text->Data());
    Ice::ObjectPrx prx = _communicator->stringToProxy("hello:tcp -h " + h + " -p 10000:ssl -h " + h + 
                                                      " -p 10001:udp -h " + h + " -p 10000");
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
        prx = prx->ice_timeout(static_cast<int>(timeout->Value * 1000));
    }
    return Demo::HelloPrx::uncheckedCast(prx);
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
        HelloPrx prx = proxy();
        if(!prx)
        {
            return;
        }

        if(!isBatch())
        {
            print("Sending sayHello request.");
            _response = false;
            hello->IsEnabled = false;
            int deliveryMode = mode->SelectedIndex;
            Ice::AsyncResultPtr result = prx->begin_sayHello(static_cast<int>(delay->Value * 1000),
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
                print("Ready");
            }
        }
        else
        {
            print("Queued hello request.");
            prx->sayHello(static_cast<int>(delay->Value * 1000));
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
    try
    {
        HelloPrx prx = proxy();
        if(!prx)
        {
            return;
        }
        prx = Demo::HelloPrx::uncheckedCast(prx->ice_twoway());
        prx->begin_shutdown();
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
MainPage::print(const std::string& message)
{
    output->Text = ref new String(IceUtil::stringToWstring(message).c_str());
}

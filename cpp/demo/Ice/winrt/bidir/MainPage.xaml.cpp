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

using namespace bidir;

using namespace std;
using namespace Demo;
using namespace Ice;
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

void
CallbackReceiverI::callback(Ice::Int num, const Ice::Current& current)
{
    _page->callback(num, current);
}

MainPage::MainPage()
{
    InitializeComponent();
}

void bidir::MainPage::startClient_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
    try
    {
        Ice::InitializationData initData;
        initData.properties = Ice::createProperties();
        initData.properties->setProperty("Ice.Warn.Connections", "1");
        initData.properties->setProperty("Ice.ACM.Client", "0");

        _communicator = Ice::initialize(initData);

        CallbackSenderPrx server = CallbackSenderPrx::checkedCast(
            _communicator->stringToProxy("sender:tcp -h " + IceUtil::wstringToString(hostname->Text->Data()) + 
                                         " -p 10000"));

        if(!server)
        {
            print("invalid proxy\n");
            return;
        }
        startClient->IsEnabled = false;
        stopClient->IsEnabled = true;
        Ice::ObjectAdapterPtr adapter = _communicator->createObjectAdapter("");
        Ice::Identity ident;
        ident.name = IceUtil::generateUUID();
        ident.category = "";
        CallbackReceiverPtr cr = new CallbackReceiverI(this);
        adapter->add(cr, ident);
        adapter->activate();
        server->ice_getConnection()->setAdapter(adapter);
        server->begin_addClient(ident, nullptr, [=](const Ice::Exception& ex)
                                                    {
                                                        ostringstream os;
                                                        os << ex << endl;
                                                        print(os.str());
                                                        startClient->IsEnabled = true;
                                                        stopClient->IsEnabled = false;
                                                    });
    }
    catch(const Ice::Exception& ex)
    {
        ostringstream os;
        os << ex << endl;
        print(os.str());
        startClient->IsEnabled = true;
        stopClient->IsEnabled = false;
    }
}


void bidir::MainPage::stopClient_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
    try
    {
        if(_communicator)
        {
            _communicator->destroy();
            _communicator = 0;
        }
        startClient->IsEnabled = true;
        stopClient->IsEnabled = false;
    }
    catch(const Ice::Exception& ex)
    {
        ostringstream os;
        os << ex << endl;
        print(os.str());
        startClient->IsEnabled = true;
        stopClient->IsEnabled = false;
    }
}

void
bidir::MainPage::callback(Ice::Int num, const Ice::Current&)
{
    ostringstream os;
    os << "received callback #" << num << endl;
    print(os.str());
}

void 
bidir::MainPage::print(const std::string& message)
{
    this->Dispatcher->RunAsync(CoreDispatcherPriority::Normal, 
                    ref new DispatchedHandler(
                            [=] ()
                                {
                                    output->Text += ref new String(IceUtil::stringToWstring(message).c_str());
                                    output->UpdateLayout();
#if (_WIN32_WINNT > 0x0602)
                                    scroller->ChangeView(nullptr, scroller->ScrollableHeight, nullptr);
#else
                                    scroller->ScrollToVerticalOffset(scroller->ScrollableHeight);
#endif
                                }, 
                            CallbackContext::Any));
}

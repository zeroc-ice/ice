//
// MainPage.xaml.cpp
// Implementation of the MainPage class.
//

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

class DispatcherI : virtual public Ice::Dispatcher
{
public:

    DispatcherI(CoreDispatcher^ dispatcher) :
        _dispatcher(dispatcher)
    {
    }

    virtual void dispatch(const Ice::DispatcherCallPtr& call, const Ice::ConnectionPtr&)
    {
        _dispatcher->RunAsync(CoreDispatcherPriority::Normal, 
                              ref new DispatchedHandler([=]()
                                    {
                                        call->run();
                                    }, CallbackContext::Any));
    }

private:

    CoreDispatcher^ _dispatcher;
};

// The Blank Page item template is documented at http://go.microsoft.com/fwlink/?LinkId=234238

MainPage::MainPage()
{
    InitializeComponent();
    mode->SelectedIndex = 0;
    Ice::InitializationData id;
    id.dispatcher = new DispatcherI(this->Dispatcher);  
    _communicator = Ice::initialize(id);
}

/// <summary>
/// Invoked when this page is about to be displayed in a Frame.
/// </summary>
/// <param name="e">Event data that describes how this page was reached.  The Parameter
/// property is typically used to configure the page.</param>
void MainPage::OnNavigatedTo(NavigationEventArgs^ e)
{
    (void) e; // Unused parameter
}

HelloCallback::HelloCallback(MainPage^ page) :
    _page(page)
{
}

void 
HelloCallback::helloSent(bool sent)
{
    _page->helloSent(sent);
}

void 
HelloCallback::helloSuccess()
{
    _page->helloSuccess();
}

void
HelloCallback::helloFailure(const Ice::Exception& ex)
{
    _page->helloFailure(ex);
}

void 
hello::MainPage::helloSuccess()
{
    print("Ready.");
}

void
hello::MainPage::helloFailure(const Ice::Exception& ex)
{
    ostringstream os;
    os << ex;
    print(os.str());
}

void
hello::MainPage::helloSent(bool)
{
    if(mode->SelectedIndex == 0 || mode->SelectedIndex == 1)
    {
        print("Waiting for response.");
    }
    else
    {
        print("Ready.");
    }
}

Demo::HelloPrx
hello::MainPage::proxy()
{
    string h = IceUtil::wstringToString(hostname->Text->Data());
    Ice::ObjectPrx prx = _communicator->stringToProxy("hello:tcp -h " + h + " -p 10000:ssl -h " + h + " -p 10001:udp -h " + h + " -p 10000");
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
        prx = prx->ice_timeout((int)timeout->Value * 1000);
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
            Demo::Callback_Hello_sayHelloPtr cb = 
                Demo::newCallback_Hello_sayHello(new HelloCallback(this), 
                                                 &HelloCallback::helloSuccess,
                                                 &HelloCallback::helloFailure, 
                                                 &HelloCallback::helloSent);
            prx->begin_sayHello(static_cast<int>(delay->Value * 1000), cb);
        }
        else
        {
            print("Queued sayHello request.");
            prx->sayHello((int)(delay->Value * 1000));
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
        Ice::Callback_Communicator_flushBatchRequestsPtr cb = 
            Ice::newCallback_Communicator_flushBatchRequests(new HelloCallback(this), &HelloCallback::helloFailure);
        _communicator->begin_flushBatchRequests(cb);
        print("Flushed batch requests.");
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

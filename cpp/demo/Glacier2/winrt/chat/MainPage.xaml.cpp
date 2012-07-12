//
// MainPage.xaml.cpp
// Implementation of the MainPage class.
//

#include "pch.h"
#include "MainPage.xaml.h"

using namespace chat;

using namespace Platform;
using namespace Windows::System;
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
using namespace Windows::UI::Xaml::Interop;

using namespace std;

// The Blank Page item template is documented at http://go.microsoft.com/fwlink/?LinkId=234238

MainPage^ MainPage::_instance = nullptr;

class DispatcherI : virtual public Ice::Dispatcher
{
public:

    DispatcherI(CoreDispatcher^ dispatcher) :
        _dispatcher(dispatcher)
    {
    }

    virtual void dispatch(const Ice::DispatcherCallPtr& call, const Ice::ConnectionPtr&)
    {
        _dispatcher->RunAsync(CoreDispatcherPriority::Normal, ref new DispatchedHandler([=]()
            {
                call->run();
            }, CallbackContext::Any));
    }

private:

    CoreDispatcher^ _dispatcher;
};

Coordinator::Coordinator(CoreDispatcher^ dispatcher) :
    _dispatcher(dispatcher)
{
}

void
Coordinator::signIn(LoginData loginData)
{
    _loginData = loginData;
    Ice::InitializationData id;
    id.properties = Ice::createProperties();
    id.dispatcher = new DispatcherI(_dispatcher);
    Glacier2::SessionFactoryHelperPtr factory = new Glacier2::SessionFactoryHelper(id, this);
    Ice::Identity identity;
    identity.name = "router";
    identity.category = "DemoGlacier2";
    factory->setRouterIdentity(identity);
    factory->setRouterHost(_loginData.hostname);
    _session = factory->connect(_loginData.username, _loginData.password);
}

LoginData
Coordinator::loginData()
{
    return _loginData;
}

void
Coordinator::say(const std::string& msg)
{
    try
    {
        Demo::Callback_ChatSession_sayPtr cb = Demo::newCallback_ChatSession_say(this, &Coordinator::sayCallbackSuccess, 
                                                                                       &Coordinator::sayCallbackError);
        _chat->begin_say(msg, cb);
    }
    catch(const Ice::CommunicatorDestroyedException& ex)
    {
        ostringstream os;
        os << "Connect failed:\n" << ex << endl; 
        MainPage::instance()->setError(os.str());
    }
}

void
Coordinator::sayCallbackSuccess()
{
}

void 
Coordinator::sayCallbackError(const Ice::Exception& ex)
{
    ostringstream os;
    os << "Connect failed:\n" << ex << endl; 
    MainPage::instance()->setError(os.str());
}

void
Coordinator::createdCommunicator(const Glacier2::SessionHelperPtr&)
{
}

void
Coordinator::setCallbackSuccess()
{
    MainPage::instance()->setConnected(true);
}

void
Coordinator::setCallbackError(const Ice::Exception& ex)
{
    ostringstream os;
    os << "Connect failed:\n" << ex << endl; 
    MainPage::instance()->setError(os.str());
}

void
Coordinator::connected(const Glacier2::SessionHelperPtr& session)
{
    if(_session != session)
    {
        return;
    }
    try
    {
        _chat = Demo::ChatSessionPrx::uncheckedCast(session->session());
        Demo::Callback_ChatSession_setCallbackPtr cb = 
            Demo::newCallback_ChatSession_setCallback(this, &Coordinator::setCallbackSuccess, &Coordinator::setCallbackError);
        _chat->begin_setCallback(Demo::ChatCallbackPrx::uncheckedCast(_session->addWithUUID(this)), cb);
    }
    catch(const Ice::CommunicatorDestroyedException& ex)
    {
        ostringstream os;
        os << "Connect failed:\n" << ex << endl; 
        MainPage::instance()->setError(os.str());
    }
}

void
Coordinator::disconnected(const Glacier2::SessionHelperPtr&)
{
    MainPage::instance()->setConnected(false);
}

void
Coordinator::connectFailed(const Glacier2::SessionHelperPtr&, const Ice::Exception& ex)
{
    ostringstream os;
    os << "Connect failed:\n" << ex << endl; 
    MainPage::instance()->setError(os.str());
}
 
void
Coordinator::message(const string& msg, const Ice::Current&)
{
    try
    {
        MainPage::instance()->_chatView->appendMessage(ref new String(IceUtil::stringToWstring(msg).c_str()));
    }
    catch(const Ice::CommunicatorDestroyedException& ex)
    {
        ostringstream os;
        os << ex << endl; 
        MainPage::instance()->setError(os.str());
    }
}

void
Coordinator::destroy()
{
    if(_session)
    {
        _session->destroy();
        _session = 0;
    }
}

MainPage::MainPage()
{
    InitializeComponent();
    _instance = this;
    _coordinator = new Coordinator(this->Dispatcher);
    setConnected(false);
}

MainPage^
MainPage::instance()
{
    return _instance;
}

void
MainPage::setConnected(bool connected)
{
    String^ pageName = nullptr;
    if(connected)
    {
        pageName = "chat.ChatView";
        signout->Visibility =  Windows::UI::Xaml::Visibility::Visible;
    }
    else
    {
        pageName = "chat.LoginView";
        signout->Visibility =  Windows::UI::Xaml::Visibility::Collapsed;
    }
    TypeName page = {pageName, TypeKind::Custom};
    main->Navigate(page, this);
    _loginView->signin->IsEnabled = true;
}

void
MainPage::setError(const std::string& err)
{
    setConnected(false);
    _loginView->setError(ref new String(IceUtil::stringToWstring(err).c_str()));
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

void chat::MainPage::signoutClick(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
    _coordinator->destroy();
}

//
// ChatView.xaml.h
// Declaration of the ChatView class
//

#pragma once

#include "ChatView.g.h"

namespace chat
{
    /// <summary>
    /// An empty page that can be used on its own or navigated to within a Frame.
    /// </summary>
    public ref class ChatView sealed
    {
    public:
        
        ChatView();

        void setError(Platform::String^ err);

        void appendMessage(Platform::String^ message);

    protected:
        
        virtual void OnNavigatedTo(Windows::UI::Xaml::Navigation::NavigationEventArgs^ e) override;
        
    private:
        
        void inputKeyDown(Platform::Object^ sender, Windows::UI::Xaml::Input::KeyRoutedEventArgs^ e);
    };
}

// JScript source code

//contains calls to silverlight.js, example below loads Page.xaml
function createSilverlight()
{
	Silverlight.createObjectEx({
		source: "Page.xaml",
		parentElement: document.getElementById("SilverlightControlHost"),
		id: "SilverlightControl",
		properties: {
			width: "100%",
			height: "100%",
			version: "1.1",
			enableHtmlAccess: "true"
		},
		events: {}
	});
	   
	// Give the keyboard focus to the Silverlight control by default
    document.body.onload = function() {
      var silverlightControl = document.getElementById('SilverlightControl');
      if (silverlightControl)
      silverlightControl.focus();
    }

}

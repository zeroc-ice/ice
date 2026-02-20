cask "${ICEGRIDGUI_CASK_NAME}" do
  version "${ICE_VERSION}"
  sha256 "${ICEGRIDGUI_DMG_SHA256}"

  url "${ICEGRIDGUI_DMG_URL}"
  name "IceGrid GUI"
  desc "Graphical administration tool for IceGrid"
  homepage "https://zeroc.com"

  app "IceGrid GUI.app"
end

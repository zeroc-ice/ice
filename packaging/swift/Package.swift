// swift-tools-version:5.0
// The swift-tools-version declares the minimum version of Swift required to build this package.

import PackageDescription

let cxxSettings: [CXXSetting] = [
    .define("ICE_BUILDING_SRC"),
    .define("ICE_STATIC_LIBS"),
    .define("ICE_SWIFT"),
    .define("ICE_CPP11_MAPPING")]

let package = Package(
  name: "Ice",
  platforms: [
    .macOS(.v10_14),
    .iOS(.v12),
  ],
  products: [
    .library(name: "Ice", targets: ["Ice"]),
    .library(name: "Glacier2", targets: ["Glacier2"]),
    .library(name: "IceStorm", targets: ["IceStorm"]),
    .library(name: "IceGrid", targets: ["IceGrid"])
  ],
  dependencies: [
    .package(url: "https://github.com/mxcl/PromiseKit.git", from: "6.8.10")
  ],
  targets: [
    .target(name: "IceCpp", cxxSettings: cxxSettings,
            linkerSettings: [.linkedLibrary("iconv"), .linkedLibrary("bz2")]),
    .target(name: "IceSSLCpp", dependencies: ["IceCpp"], cxxSettings: cxxSettings),
    .target(name: "IceDiscoveryCpp", dependencies: ["IceCpp"], cxxSettings: cxxSettings),
    .target(name: "IceLocatorDiscoveryCpp", dependencies: ["IceCpp"], cxxSettings: cxxSettings),
    .target(name: "IceImpl",
            dependencies: ["IceCpp", "IceSSLCpp", "IceDiscoveryCpp", "IceLocatorDiscoveryCpp"],
            cxxSettings: cxxSettings,
            linkerSettings: [.linkedFramework("ExternalAccessory")]),
    .target(name: "Ice", dependencies: ["IceImpl", "PromiseKit"]),
    .target(name: "Glacier2", dependencies: ["Ice"]),
    .target(name: "IceStorm", dependencies: ["Ice"]),
    .target(name: "IceGrid", dependencies: ["Ice", "Glacier2"])
  ],
  cxxLanguageStandard: .cxx14
)

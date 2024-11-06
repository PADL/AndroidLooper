// swift-tools-version: 6.0
// The swift-tools-version declares the minimum version of Swift required to build this package.

import PackageDescription

let package = Package(
  name: "AndroidLooper",
  platforms: [
    .macOS(.v10_15),
  ],
  products: [
    // Products define the executables and libraries a package produces, making them visible to
    // other packages.
    .library(
      name: "AndroidLooper",
      targets: ["AndroidLooper"]
    ),
  ],
  dependencies: [
    .package(url: "https://github.com/apple/swift-system", from: "1.4.0"),
    .package(
      url: "https://github.com/skiptools/swift-android-native",
      branch: "main"
    ),
  ],
  targets: [
    .target(
      name: "CAndroidLooper",
      linkerSettings: [.linkedLibrary("android")]
    ),
    .target(
      name: "AndroidLooper",
      dependencies: [
        "CAndroidLooper",
        .product(name: "SystemPackage", package: "swift-system"),
        .product(name: "AndroidLogging", package: "swift-android-native"),
      ],
      swiftSettings: [.swiftLanguageMode(.v5)]
    ),
  ]
)

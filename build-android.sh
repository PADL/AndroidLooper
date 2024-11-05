#!/bin/sh

set -Eeu

pwd=`pwd`

NDK_VERS=24

SWIFT_VERS=6.0.2
SWIFT_SDK="$(swift sdk list|grep android)"
SWIFT_SDK_SYSROOT="${HOME}/.swiftpm/swift-sdks/${SWIFT_SDK}.artifactbundle/swift-${SWIFT_VERS}-release-android-${NDK_VERS}-sdk/android-27c-sysroot"

TOOLCHAINS="/Library/Developer/Toolchains/swift-${SWIFT_VERS}-RELEASE.xctoolchain"
export TOOLCHAINS

TRIPLE="aarch64-unknown-linux-android${NDK_VERS}"
export TRIPLE

swift build --toolchain ${TOOLCHAINS} --swift-sdk ${TRIPLE}

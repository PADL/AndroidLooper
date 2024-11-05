//
// Copyright (c) 2024 PADL Software Pty Ltd
//
// Licensed under the Apache License, Version 2.0 (the License);
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an 'AS IS' BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

#pragma once

#include <stdbool.h>

#include <android/looper.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef __attribute__((__swift_attr__("@Sendable"))) void (
    ^CAndroidLooperCallbackBlock)(void);

/// register a block to be invoked when the file descriptor is signalled. if
/// oneShot is true, then the block is invoked once and removed; otherwise, it
/// is invoked each time the fd is signalled.
int CAndroidLooper_setBlock(ALooper *_Nonnull looper,
                            int fd,
                            _Nullable CAndroidLooperCallbackBlock block,
                            bool oneShot);

/// logging helper, to be removed
void CAndroidLooper_log(ALooper *_Nullable looper, const char *_Nonnull msg);

/// return the looper that was registered when the library was loaded
ALooper *_Nullable CAndroidLooper_getMainLooper(void);

/// return the JNI environment that was present when the library was loaded
JNIEnv *_Nullable CAndroidLooper_getMainEnvironment(void);

#ifdef __cplusplus
}
#endif

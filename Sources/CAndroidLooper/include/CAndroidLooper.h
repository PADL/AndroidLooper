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

#include <android/looper.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef __attribute__((__swift_attr__("@Sendable"))) void (
    ^CAndroidLooperCallbackBlock)(void);

int CAndroidLooper_setBlock(ALooper *_Nonnull looper,
                            int fd,
                            _Nullable CAndroidLooperCallbackBlock block);

void CAndroidLooper_log(ALooper *_Nullable looper, const char *_Nonnull msg);

ALooper *_Nullable CAndroidLooper_getMainLooper(void);

#ifdef __cplusplus
}
#endif

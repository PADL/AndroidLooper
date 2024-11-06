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

#include <cstring>

#include <pthread.h>
#include <android/log.h>

#include "CAndroidLooper.h"

void CAndroidLooper_log(ALooper *_Nullable looper, const char *_Nonnull msg) {
  if (looper)
    __android_log_print(ANDROID_LOG_DEBUG, "CAndroidLooper",
                        "thread %lx looper %p -- %s", pthread_self(), looper,
                        msg);
  else
    __android_log_print(ANDROID_LOG_DEBUG, "CAndroidLooper", "thread %lx -- %s",
                        pthread_self(), msg);
}

//
// Copyright (c) 2023-2024 PADL Software Pty Ltd
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

#include <unistd.h>

#include <map>
#include <mutex>

#include "Block.hpp"

#include <android/log.h>
#include <jni.h>

#include "CAndroidLooper.h"

extern "C" {
static int CAndroidLooper_callbackFunc(int fd, int events, void *data);
JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *jvm, void *reserved);
}

static ALooper *CAndroidLooper_uiThread;
static std::map<int, Block<void>> CAndroidLooper_blocks{};
static std::mutex CAndroidLooper_mutex;

// FIXME: assumes that library is always loaded on the UI thread

__attribute__((__constructor__)) static void CAndroidLooper_init(void) {
  CAndroidLooper_uiThread = ALooper_forThread();
  ALooper_acquire(CAndroidLooper_uiThread);
}

__attribute__((__destructor__)) static void CAndroidLooper_deinit(void) {
  if (CAndroidLooper_uiThread) {
    ALooper_release(CAndroidLooper_uiThread);
    CAndroidLooper_uiThread = nullptr;
  }
}

static int CAndroidLooper_callbackFunc(int fd, int events, void *data) {
  CAndroidLooperCallbackBlock block =
      reinterpret_cast<CAndroidLooperCallbackBlock>(data);
  block();
  return 1;
}

typedef int (*ALooper_callbackFunc)(int fd, int events, void *data);

int CAndroidLooper_setBlock(ALooper *looper,
                            int fd,
                            CAndroidLooperCallbackBlock block) {
  std::lock_guard<std::mutex> guard(CAndroidLooper_mutex);
  int err;

  if (block) {
    err = ALooper_addFd(looper, fd, ALOOPER_POLL_CALLBACK, ALOOPER_EVENT_INPUT,
                        CAndroidLooper_callbackFunc, block);
    if (err == 1)
      CAndroidLooper_blocks.emplace(std::make_pair(fd, Block(block)));
  } else {
    err = ALooper_removeFd(looper, fd);
    if (err == 1)
      CAndroidLooper_blocks.erase(fd);
  }

  return err;
}

void CAndroidLooper_log(ALooper *_Nullable looper, const char *_Nonnull msg) {
  if (looper)
    __android_log_print(ANDROID_LOG_DEBUG, "CAndroidLooper", "[%p] %s\n",
                        looper, msg);
  else
    __android_log_print(ANDROID_LOG_DEBUG, "CAndroidLooper", "%s\n", msg);
}

static ALooper *_Nullable CAndroidLooper_mainLooper;

ALooper *_Nullable CAndroidLooper_getMainLooper(void) {
  return CAndroidLooper_mainLooper;
}

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *jvm, void *reserved) {
  if (CAndroidLooper_mainLooper)
    ALooper_release(CAndroidLooper_mainLooper);
  CAndroidLooper_mainLooper = ALooper_forThread();
  ALooper_acquire(CAndroidLooper_mainLooper);

  return JNI_VERSION_1_6;
}

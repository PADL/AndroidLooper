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

#include <unistd.h>

#include <map>
#include <mutex>

#include "Block.hpp"

#include <android/log.h>
#include <jni.h>
#include <pthread.h>

#include "CAndroidLooper.h"

extern "C" {
static int CAndroidLooper_callbackThunk(int fd, int events, void *data);
static int CAndroidLooper_callbackThunkOneShot(int fd, int events, void *data);

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *jvm, void *reserved);
JNIEXPORT void JNICALL JNI_OnUnload(JavaVM *pJavaVM, void *pReserved);
}

static std::map<int, Block<void>> CAndroidLooper_blocks{};
static std::mutex CAndroidLooper_mutex;

static int CAndroidLooper_callbackThunk(int fd, int events, void *data) {
  auto block = reinterpret_cast<CAndroidLooperCallbackBlock>(data);

  if (block)
    block();

  return 1;
}

static int CAndroidLooper_callbackThunkOneShot(int fd, int events, void *data) {
  auto block = reinterpret_cast<CAndroidLooperCallbackBlock>(data);

  if (block) {
    block();
    _Block_release(block);
  }

  return 0;
}

int CAndroidLooper_setBlock(ALooper *looper,
                            int fd,
                            CAndroidLooperCallbackBlock block,
                            bool oneShot) {
  std::lock_guard<std::mutex> guard(CAndroidLooper_mutex);
  int err;

  if (oneShot) {
    err =
        ALooper_addFd(looper, fd, ALOOPER_POLL_CALLBACK, ALOOPER_EVENT_INPUT,
                      CAndroidLooper_callbackThunkOneShot, _Block_copy(block));
  } else if (block) {
    err = ALooper_addFd(looper, fd, ALOOPER_POLL_CALLBACK, ALOOPER_EVENT_INPUT,
                        CAndroidLooper_callbackThunk, block);
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
    __android_log_print(ANDROID_LOG_DEBUG, "CAndroidLooper",
                        "thread %lx looper %p -- %s", pthread_self(), looper,
                        msg);
  else
    __android_log_print(ANDROID_LOG_DEBUG, "CAndroidLooper", "thread %lx -- %s",
                        pthread_self(), msg);
}

static ALooper *_Nullable CAndroidLooper_mainLooper;
static JNIEnv *_Nullable CAndroidLooper_mainEnvironment;

ALooper *_Nullable CAndroidLooper_getMainLooper(void) {
  return CAndroidLooper_mainLooper;
}

JNIEnv *_Nullable CAndroidLooper_getMainEnvironment(void) {
  return CAndroidLooper_mainEnvironment;
}

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *jvm, void *reserved) {
  jvm->GetEnv(reinterpret_cast<void **>(&CAndroidLooper_mainEnvironment),
              JNI_VERSION_1_6);

  if (CAndroidLooper_mainLooper)
    ALooper_release(CAndroidLooper_mainLooper);
  CAndroidLooper_mainLooper = ALooper_forThread();
  ALooper_acquire(CAndroidLooper_mainLooper);

  CAndroidLooper_log(nullptr, "loaded Swift AndroidLooper library");
  return JNI_VERSION_1_6;
}

JNIEXPORT void JNI_OnUnload(JavaVM *pJavaVM, void *pReserved) {
  if (CAndroidLooper_mainLooper) {
    ALooper_release(CAndroidLooper_mainLooper);
    CAndroidLooper_mainLooper = nullptr;
  }

  std::lock_guard<std::mutex> guard(CAndroidLooper_mutex);
  CAndroidLooper_blocks.clear();

  CAndroidLooper_mainEnvironment = nullptr;

  CAndroidLooper_log(nullptr, "unloaded Swift AndroidLooper library");
}

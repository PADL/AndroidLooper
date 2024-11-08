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

import Android
import CAndroidLooper

private var _mainLooper: OpaquePointer?

public extension ALooper {
  static var sharedUIThreadLooper: Self {
    Self(wrapping: _mainLooper!)
  }
}

public final class UIThreadExecutor: AExecutor, @unchecked Sendable {
  public convenience init() {
    try! self.init(looper: ALooper.sharedUIThreadLooper)
  }
}

@globalActor
public final actor UIThreadActor: GlobalActor {
  // ensure executor is retained to avoid crash
  // https://forums.swift.org/t/how-to-properly-use-custom-executor-on-global-actor/71829/4
  private static let _executor = UIThreadExecutor()

  public static let shared = UIThreadActor()
  public static let sharedUnownedExecutor: UnownedSerialExecutor = UIThreadActor._executor
    .asUnownedSerialExecutor()

  public nonisolated var unownedExecutor: UnownedSerialExecutor {
    Self.sharedUnownedExecutor
  }
}

// call from your applications JNI_OnLoad
public func AndroidLooper_initialize(_ reserved: UnsafeRawPointer?) {
  if _mainLooper != nil {
    ALooper_release(_mainLooper)
  }
  _mainLooper = ALooper_forThread()
  ALooper_acquire(_mainLooper)
}

public func AndroidLooper_deinitialize(_ reserved: UnsafeRawPointer?) {
  ALooper_release(_mainLooper)
  _mainLooper = nil
}

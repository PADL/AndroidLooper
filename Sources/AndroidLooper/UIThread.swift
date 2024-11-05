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

public extension ALooper {
  static var sharedUIThreadLooper: Self {
    Self(wrapping: CAndroidLooper_getMainLooper()!)
  }
}

public final class UIThreadExecutor: AExecutor, @unchecked Sendable {
  public convenience init() {
    self.init(looper: ALooper.sharedUIThreadLooper)!
  }
}

@globalActor
public final actor UIThreadActor: GlobalActor {
  public static let shared = UIThreadActor()
  public static let sharedUnownedExecutor: UnownedSerialExecutor = UIThreadExecutor()
    .asUnownedSerialExecutor()

  public nonisolated var unownedExecutor: UnownedSerialExecutor {
    Self.sharedUnownedExecutor
  }
}

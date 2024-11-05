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

import Android
import CAndroidLooper

public enum ALooperError: Error {
  case initializationFailure
  case enqueueJobFailure
}

public struct ALooper: ~Copyable, @unchecked Sendable {
  public typealias Block = @Sendable () -> ()

  private let _looper: OpaquePointer

  public init(wrapping looper: OpaquePointer) {
    ALooper_acquire(looper)
    _looper = looper
  }

  deinit {
    ALooper_release(_looper)
  }

  public func set(fd: CInt, _ block: Block?) throws {
    if CAndroidLooper_setBlock(_looper, fd, block) < 0 {
      throw ALooperError.initializationFailure
    }
  }

  public static func prepare(opts: CInt) throws -> Self {
    guard let looper = ALooper_prepare(opts) else {
      throw ALooperError.initializationFailure
    }
    return ALooper(wrapping: looper)
  }

  public func wake() {
    ALooper_wake(_looper)
  }

  @discardableResult
  public func remove(fd: CInt) throws -> Bool {
    let ret = ALooper_removeFd(_looper, fd)
    if ret < 0 {
      throw ALooperError.initializationFailure
    }
    return ret == 1
  }

  func log(_ msg: String) {
    CAndroidLooper_log(_looper, msg)
  }
}

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
import SystemPackage

public struct ALooper: ~Copyable, @unchecked Sendable {
  public enum LooperError: Error {
    case setBlockFailure
    case removeBlockFailure
    case preparationFailure(CInt)
  }

  public typealias Block = @Sendable () -> ()

  private let _looper: OpaquePointer

  public init(wrapping looper: OpaquePointer) {
    ALooper_acquire(looper)
    _looper = looper
  }

  deinit {
    ALooper_release(_looper)
  }

  public func set(fd: FileDescriptor, _ block: Block?) throws {
    if CAndroidLooper_setBlock(_looper, fd.rawValue, block) < 0 {
      throw LooperError.setBlockFailure
    }
  }

  public static func prepare(opts: CInt) throws -> Self {
    guard let looper = ALooper_prepare(opts) else {
      throw LooperError.preparationFailure(opts)
    }
    return ALooper(wrapping: looper)
  }

  public func wake() {
    ALooper_wake(_looper)
  }

  @discardableResult
  public func remove(fd: FileDescriptor) throws -> Bool {
    let ret = ALooper_removeFd(_looper, fd.rawValue)
    if ret < 0 {
      throw LooperError.removeBlockFailure
    }
    return ret == 1
  }

  func log(_ msg: String) {
    CAndroidLooper_log(_looper, msg)
  }
}

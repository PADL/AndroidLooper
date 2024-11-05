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

private extension Duration {
  var milliseconds: Double {
    Double(components.seconds) * 1000 + Double(components.attoseconds) * 1e-15
  }
}

public struct ALooper: ~Copyable, @unchecked Sendable {
  public enum LooperError: Error {
    case setBlockFailure
    case removeBlockFailure
    case preparationFailure(CInt)
    case pollTimeout
    case pollError
  }

  public struct Events: OptionSet, Sendable {
    public typealias RawValue = CInt

    public let rawValue: RawValue

    public init(rawValue: RawValue) {
      self.rawValue = rawValue
    }

    public static let input = Events(rawValue: 1 << 0)
    public static let output = Events(rawValue: 1 << 1)
    public static let error = Events(rawValue: 1 << 2)
    public static let hangup = Events(rawValue: 1 << 3)
    public static let invalid = Events(rawValue: 1 << 4)
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

  public func set(fd: FileDescriptor, _ block: Block?, oneShot: Bool = false) throws {
    if CAndroidLooper_setBlock(_looper, fd.rawValue, block, oneShot) < 0 {
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

  public struct PollResult {
    let ident: CInt
    let fd: CInt
    let events: Events
    let data: UnsafeRawPointer?
  }

  public static func pollOnce(duration: Duration? = nil) throws -> PollResult? {
    var outFd: CInt = -1
    var outEvents: CInt = 0
    var outData: UnsafeMutableRawPointer?

    let timeoutMillis = CInt(duration?.milliseconds ?? 0)
    let err = ALooper_pollOnce(timeoutMillis, &outFd, &outEvents, &outData)
    switch Int(err) {
    case ALOOPER_POLL_WAKE:
      fallthrough
    case ALOOPER_POLL_CALLBACK:
      return nil
    case ALOOPER_POLL_TIMEOUT:
      throw LooperError.pollTimeout
    case ALOOPER_POLL_ERROR:
      throw LooperError.pollError
    default:
      return PollResult(ident: err, fd: outFd, events: Events(rawValue: outEvents), data: outData)
    }
  }

  func log(_ msg: String) {
    CAndroidLooper_log(_looper, msg)
  }
}

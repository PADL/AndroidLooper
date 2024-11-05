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

// Swift structured concurrency executor that enqueues jobs on an Android
// Looper.
open class AExecutor: SerialExecutor, @unchecked Sendable {
  private let _eventFd: FileDescriptor
  private let _looper: ALooper
  private let _queue = LockedState(initialState: [UnownedJob]())

  /// Initialize with Android Looper
  public init?(looper: consuming ALooper) {
    let fd = eventfd(0, EFD_CLOEXEC | EFD_NONBLOCK)
    if fd < 0 {
      return nil
    }
    _eventFd = FileDescriptor(rawValue: fd)
    _looper = looper
    do {
      try _looper.set(fd: _eventFd) { self.drain() }
    } catch {
      try? _eventFd.close()
      return nil
    }
  }

  deinit {
    if _eventFd.rawValue != -1 {
      try? _looper.set(fd: _eventFd, nil)
      try? _eventFd.close()
    }
  }

  /// Read number of remaining events from eventFd
  private var eventsRemaining: UInt64 {
    var value = UInt64(0)

    do {
      try withUnsafeMutableBytes(of: &value) {
        guard try _eventFd.read(into: $0) == MemoryLayout<UInt64>.size else {
          throw Errno.invalidArgument
        }
      }
    } catch {
      value = 0
    }

    return value
  }

  /// Increment number of remaining events on eventFd
  private func signal() throws {
    var value = UInt64(1)

    try withUnsafeBytes(of: &value) {
      guard try _eventFd.write($0) == MemoryLayout<UInt64>.size else {
        throw Errno.outOfRange
      }
    }
  }

  /// Drain job queue
  private func drain() {
    let eventsRemaining = eventsRemaining
    for _ in 0..<eventsRemaining {
      let job = dequeue()
      guard let job else { break }
      job.runSynchronously(on: asUnownedSerialExecutor())
    }
  }

  /// Dequeue a single job
  private func dequeue() -> UnownedJob? {
    _queue.withLock { queue in
      guard !queue.isEmpty else { return nil }
      return queue.removeFirst()
    }
  }

  /// Enqueue a single job
  public func enqueue(_ job: UnownedJob) {
    _queue.withLock { queue in
      queue.append(job)
    }
    try! signal()
  }

  public func asUnownedSerialExecutor() -> UnownedSerialExecutor {
    UnownedSerialExecutor(ordinary: self)
  }
}

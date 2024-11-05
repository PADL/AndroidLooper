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

open class AExecutor: SerialExecutor, @unchecked Sendable {
  private let _eventFd: CInt
  private let _looper: ALooper
  private let _queue = LockedState(initialState: [UnownedJob]())

  public init?(looper: consuming ALooper) {
    _eventFd = eventfd(0, EFD_CLOEXEC | EFD_NONBLOCK)
    if _eventFd < 0 {
      return nil
    }
    _looper = looper
    do {
      try _looper.set(fd: _eventFd) { self.drain() }
    } catch {
      close(_eventFd)
      return nil
    }
  }

  deinit {
    if _eventFd != -1 {
      try? _looper.set(fd: _eventFd, nil)
      close(_eventFd)
    }
  }

  private var eventsRemaining: UInt64 {
    var value = UInt64(0)

    if read(_eventFd, &value, MemoryLayout<UInt64>.size) != MemoryLayout<UInt64>.size {
      return 0
    }

    return value
  }

  // increments eventsRemaining by the counter value
  private func signal() throws {
    var value = UInt64(1)
    guard write(_eventFd, &value, MemoryLayout<UInt64>.size) == MemoryLayout<UInt64>.size else {
      throw ALooperError.enqueueJobFailure
    }
  }

  private func drain() {
    let eventsRemaining = eventsRemaining
    for _ in 0..<eventsRemaining {
      let job = dequeue()
      guard let job else { break }
      job.runSynchronously(on: asUnownedSerialExecutor())
    }
  }

  private func dequeue() -> UnownedJob? {
    _queue.withLock { queue in
      guard !queue.isEmpty else { return nil }
      return queue.removeFirst()
    }
  }

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

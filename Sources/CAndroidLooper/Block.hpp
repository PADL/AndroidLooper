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

#pragma once

#if __has_include(<Block.h>)
#include <Block.h>
#elif __has_include(<Block/Block.h>)
#include <Block/Block.h>
#else
extern "C" void *_Block_copy(const void *);
extern "C" void _Block_release(const void *);
#endif

template <typename Ret, typename... Args> class Block final {
  typedef Ret (^BlockType)(Args...);

public:
  Block(BlockType const &block) noexcept {
    if (block)
      ::_Block_copy(block);
    _block = block;
  }

  ~Block() {
    if (_block)
      ::_Block_release(_block);
  }

  Block &operator=(const Block &block) noexcept {
    if (block._block)
      ::_Block_copy(block._block);
    if (_block)
      ::_Block_release(_block);
    _block = block._block;

    return *this;
  }

  Block(const Block &block) noexcept {
    if (block._block)
      ::_Block_copy(block._block);
    _block = block._block;
  }

  Block(Block &&dyingObj) noexcept { *this = std::move(dyingObj); }

  Block &operator=(Block &&dyingObj) noexcept {
    if (_block)
      ::_Block_release(_block);
    _block = dyingObj._block;
    dyingObj._block = nullptr;

    return *this;
  }

  BlockType get() const { return _block; }
  BlockType operator->() const { return _block; }

  operator const void *() const noexcept { return static_cast<void *>(_block); }

  Ret operator()(Args &&...arg) const noexcept {
    return _block(std::forward<Args>(arg)...);
  }

private:
  BlockType _block;
};

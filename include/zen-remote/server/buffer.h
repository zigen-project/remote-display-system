#pragma once

#include <zen-remote/loop.h>

#include <memory>

namespace zen::remote::server {

struct IBuffer {
  virtual ~IBuffer() = default;

  virtual void *begin_access() = 0;

  virtual bool end_access() = 0;
};

/**
 * Until `on_release` is called, the data must be available.
 *
 * @param data is a pointer where the data is stored
 * @param on_release is an asynchronous callback that is called when the data is
 * no longer used by zen-remote. This callback is called by the loop
 */
std::unique_ptr<IBuffer> CreateBuffer(std::function<void *()> on_begin_access,
    std::function<bool()> on_end_access, std::function<void()> on_release,
    std::unique_ptr<ILoop> loop);

}  // namespace zen::remote::server

#include "server/buffer.h"

#include "core/logger.h"

namespace zen::remote::server {

Buffer::Buffer(std::function<void *()> on_begin_access,
    std::function<bool()> on_end_access, std::function<void()> on_release,
    std::unique_ptr<ILoop> loop)
    : on_begin_access_(on_begin_access),
      on_end_access_(on_end_access),
      on_release_(on_release),
      loop_(std::move(loop))
{
  pipe_[0] = 0;
  pipe_[1] = 0;
}

Buffer::~Buffer()
{
  uint8_t buf = 0;
  if (pipe_[1] != 0) {
    write(pipe_[1], &buf, sizeof(uint8_t));
  }

  // close pipe fds after on_release is called
}

bool
Buffer::Init()
{
  if (pipe2(pipe_, O_CLOEXEC | O_NONBLOCK) == -1) return false;

  auto delete_event_source = new FdSource();

  delete_event_source->fd = pipe_[0];
  delete_event_source->mask = FdSource::kReadable;
  delete_event_source->callback = [delete_event_source,
                                      on_release = on_release_, loop = loop_,
                                      pipe_out = pipe_[0], pipe_in = pipe_[1]](
                                      int /*fd*/, uint32_t /*mask*/) {
    on_release();

    close(pipe_out);
    close(pipe_in);
    loop->RemoveFd(delete_event_source);
    delete delete_event_source;
  };

  loop_->AddFd(delete_event_source);

  return true;
}

void *
Buffer::begin_access()
{
  return on_begin_access_();
}

bool
Buffer::end_access()
{
  return on_end_access_();
}

std::unique_ptr<IBuffer>
CreateBuffer(std::function<void *()> on_begin_access,
    std::function<bool()> on_end_access, std::function<void()> on_release,
    std::unique_ptr<ILoop> loop)
{
  auto buffer = std::make_unique<Buffer>(
      on_begin_access, on_end_access, on_release, std::move(loop));

  if (buffer->Init() == false) {
    return nullptr;
  }

  return buffer;
}

}  // namespace zen::remote::server

#include "client/service/gl-base-technique.h"

#include "client/resource-pool.h"
#include "client/service/async-session-service-caller.h"

namespace zen::remote::client::service {

GlBaseTechniqueServiceImpl::GlBaseTechniqueServiceImpl(Remote* remote)
    : remote_(remote)
{
}

void
GlBaseTechniqueServiceImpl::Register(grpc::ServerBuilder& builder)
{
  builder.RegisterService(&async_);
}

void
GlBaseTechniqueServiceImpl::Listen(
    grpc::ServerCompletionQueue* completion_queue)
{
  AsyncSessionServiceCaller<&GlBaseTechniqueService::AsyncService::RequestNew,
      &GlBaseTechniqueServiceImpl::New>::Listen(&async_, this, completion_queue,
      remote_);

  AsyncSessionServiceCaller<
      &GlBaseTechniqueService::AsyncService::RequestDelete,
      &GlBaseTechniqueServiceImpl::Delete>::Listen(&async_, this,
      completion_queue, remote_);

  AsyncSessionServiceCaller<
      &GlBaseTechniqueService::AsyncService::RequestGlDrawArrays,
      &GlBaseTechniqueServiceImpl::GlDrawArrays>::Listen(&async_, this,
      completion_queue, remote_);
}

grpc::Status
GlBaseTechniqueServiceImpl::New(grpc::ServerContext* /*context*/,
    const NewGlBaseTechniqueRequest* request, EmptyResponse* /*response*/)
{
  auto pool = remote_->session_manager()->current()->pool();
  auto gl_base_technique = std::make_shared<GlBaseTechnique>(
      request->id(), pool->update_rendering_queue());
  auto rendering_unit =
      pool->rendering_units()->Get(request->rendering_unit_id());

  rendering_unit->SetGlBaseTechnique(gl_base_technique);

  pool->gl_base_techniques()->Add(std::move(gl_base_technique));

  return grpc::Status::OK;
}

grpc::Status
GlBaseTechniqueServiceImpl::Delete(grpc::ServerContext* /*context*/,
    const DeleteResourceRequest* request, EmptyResponse* /*response*/)
{
  auto pool = remote_->session_manager()->current()->pool();
  pool->gl_base_techniques()->ScheduleRemove(request->id());
  return grpc::Status::OK;
}

grpc::Status
GlBaseTechniqueServiceImpl::GlDrawArrays(grpc::ServerContext* /*context*/,
    const GlDrawArraysRequest* request, EmptyResponse* /*response*/)
{
  auto pool = remote_->session_manager()->current()->pool();
  auto base_technique = pool->gl_base_techniques()->Get(request->id());

  base_technique->GlDrawArrays(
      request->mode(), request->first(), request->count());

  return grpc::Status::OK;
}

}  // namespace zen::remote::client::service

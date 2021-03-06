// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/common/gpu/gpu_memory_manager.h"

#if defined(ENABLE_GPU)

#include <algorithm>

#include "base/bind.h"
#include "base/message_loop.h"
#include "content/common/gpu/gpu_command_buffer_stub.h"
#include "content/common/gpu/gpu_memory_allocation.h"

namespace {

// These are predefined values (in bytes) for
// GpuMemoryAllocation::gpuResourceSizeInBytes.  Currently, the value is only
// used to check if it is 0 or non-0.  In the future, these values will not
// come from constants, but rather will be distributed dynamically.
enum {
  kResourceSizeNonHibernatedTab = 1,
  kResourceSizeHibernatedTab = 0
};

bool IsInSameContextShareGroupAsAnyOf(
    const GpuCommandBufferStubBase* stub,
    const std::vector<GpuCommandBufferStubBase*>& stubs) {
  for (std::vector<GpuCommandBufferStubBase*>::const_iterator it =
      stubs.begin(); it != stubs.end(); ++it) {
    if (stub->IsInSameContextShareGroup(**it))
      return true;
  }
  return false;
}

}

GpuMemoryManager::GpuMemoryManager(GpuMemoryManagerClient* client,
        size_t max_surfaces_with_frontbuffer_soft_limit)
    : client_(client),
      manage_scheduled_(false),
      max_surfaces_with_frontbuffer_soft_limit_(
          max_surfaces_with_frontbuffer_soft_limit),
      weak_factory_(ALLOW_THIS_IN_INITIALIZER_LIST(this)) {
}

GpuMemoryManager::~GpuMemoryManager() {
}

bool GpuMemoryManager::StubWithSurfaceComparator::operator()(
    GpuCommandBufferStubBase* lhs,
    GpuCommandBufferStubBase* rhs) {
  DCHECK(lhs->has_surface_state() && rhs->has_surface_state());
  const GpuCommandBufferStubBase::SurfaceState& lhs_ss = lhs->surface_state();
  const GpuCommandBufferStubBase::SurfaceState& rhs_ss = rhs->surface_state();
  if (lhs_ss.visible)
    return !rhs_ss.visible || (lhs_ss.last_used_time > rhs_ss.last_used_time);
  else
    return !rhs_ss.visible && (lhs_ss.last_used_time > rhs_ss.last_used_time);
};

void GpuMemoryManager::ScheduleManage() {
  if (manage_scheduled_)
    return;
  MessageLoop::current()->PostTask(
    FROM_HERE,
    base::Bind(&GpuMemoryManager::Manage, weak_factory_.GetWeakPtr()));
  manage_scheduled_ = true;
}

// The current Manage algorithm simply classifies contexts (stubs) into
// "foreground", "background", or "hibernated" categories.
// For each of these three categories, there are predefined memory allocation
// limits and front/backbuffer states.
//
// Stubs may or may not have a surfaces, and the rules are different for each.
//
// The rules for categorizing contexts with a surface are:
//  1. Foreground: All visible surfaces.
//                 * Must have both front and back buffer.
//
//  2. Background: Non visible surfaces, which have not surpassed the
//                 max_surfaces_with_frontbuffer_soft_limit_ limit.
//                 * Will have only a frontbuffer.
//
//  3. Hibernated: Non visible surfaces, which have surpassed the
//                 max_surfaces_with_frontbuffer_soft_limit_ limit.
//                 * Will not have either buffer.
//
// The considerations for categorizing contexts without a surface are:
//  1. These contexts do not track {visibility,last_used_time}, so cannot
//     sort them directly.
//  2. These contexts may be used by, and thus affect, other contexts, and so
//     cannot be less visible than any affected context.
//  3. Contexts belong to share groups within which resources can be shared.
//
// As such, the rule for categorizing contexts without a surface is:
//  1. Find the most visible context-with-a-surface within each
//     context-without-a-surface's share group, and inherit its visibilty.
void GpuMemoryManager::Manage() {
  // Set up three allocation values for the three possible stub states
  const GpuMemoryAllocation all_buffers_allocation(
      kResourceSizeNonHibernatedTab, true, true);
  const GpuMemoryAllocation front_buffers_allocation(
      kResourceSizeNonHibernatedTab, false, true);
  const GpuMemoryAllocation no_buffers_allocation(
      kResourceSizeHibernatedTab, false, false);

  manage_scheduled_ = false;

  // Create stub lists by separating out the two types received from client
  std::vector<GpuCommandBufferStubBase*> stubs_with_surface;
  std::vector<GpuCommandBufferStubBase*> stubs_without_surface;
  {
    std::vector<GpuCommandBufferStubBase*> stubs;
    client_->AppendAllCommandBufferStubs(stubs);

    for (std::vector<GpuCommandBufferStubBase*>::iterator it = stubs.begin();
        it != stubs.end(); ++it) {
      GpuCommandBufferStubBase* stub = *it;
      if (stub->has_surface_state())
        stubs_with_surface.push_back(stub);
      else
        stubs_without_surface.push_back(stub);
    }
  }

  // Sort stubs with surface into {visibility,last_used_time} order using
  // custom comparator
  std::sort(stubs_with_surface.begin(),
            stubs_with_surface.end(),
            StubWithSurfaceComparator());
  DCHECK(std::unique(stubs_with_surface.begin(), stubs_with_surface.end()) ==
         stubs_with_surface.end());

  // Separate stubs into memory allocation sets.
  std::vector<GpuCommandBufferStubBase*> all_buffers, front_buffers, no_buffers;

  for (size_t i = 0; i < stubs_with_surface.size(); ++i) {
    GpuCommandBufferStubBase* stub = stubs_with_surface[i];
    DCHECK(stub->has_surface_state());
    if (stub->surface_state().visible) {
      all_buffers.push_back(stub);
      stub->SetMemoryAllocation(all_buffers_allocation);
    } else if (i < max_surfaces_with_frontbuffer_soft_limit_) {
      front_buffers.push_back(stub);
      stub->SetMemoryAllocation(front_buffers_allocation);
    } else {
      no_buffers.push_back(stub);
      stub->SetMemoryAllocation(no_buffers_allocation);
    }
  }

  // Now, go through the stubs without surfaces and deduce visibility using the
  // visibility of stubs which are in the same context share group.
  for (std::vector<GpuCommandBufferStubBase*>::const_iterator it =
      stubs_without_surface.begin(); it != stubs_without_surface.end(); ++it) {
    GpuCommandBufferStubBase* stub = *it;
    DCHECK(!stub->has_surface_state());
    if (IsInSameContextShareGroupAsAnyOf(stub, all_buffers)) {
      stub->SetMemoryAllocation(all_buffers_allocation);
    } else if (IsInSameContextShareGroupAsAnyOf(stub, front_buffers)) {
      stub->SetMemoryAllocation(front_buffers_allocation);
    } else {
      stub->SetMemoryAllocation(no_buffers_allocation);
    }
  }
}

#endif

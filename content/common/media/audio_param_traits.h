// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_COMMON_MEDIA_AUDIO_AUDIO_PARAM_TRAITS_H_
#define CONTENT_COMMON_MEDIA_AUDIO_AUDIO_PARAM_TRAITS_H_
#pragma once

#include "ipc/ipc_message.h"
#include "ipc/ipc_param_traits.h"

class AudioParameters;

namespace IPC {

template <>
struct ParamTraits<AudioParameters> {
  typedef AudioParameters param_type;
  static void Write(Message* m, const param_type& p);
  static bool Read(const Message* m, PickleIterator* iter, param_type* r);
  static void Log(const param_type& p, std::string* l);
};

} // namespace IPC

#endif  // CONTENT_COMMON_MEDIA_AUDIO_AUDIO_PARAM_TRAITS_H_

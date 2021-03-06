// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "remoting/client/mouse_input_filter.h"

#include "remoting/proto/event.pb.h"

namespace remoting {

MouseInputFilter::MouseInputFilter(protocol::InputStub* input_stub)
    : input_stub_(input_stub) {
  input_max_.setEmpty();
  output_max_.setEmpty();
}

MouseInputFilter::~MouseInputFilter() {
}

void MouseInputFilter::InjectKeyEvent(const protocol::KeyEvent& event) {
  input_stub_->InjectKeyEvent(event);
}

void MouseInputFilter::InjectMouseEvent(const protocol::MouseEvent& event) {
  if (input_max_.isEmpty() || output_max_.isEmpty())
    return;

  // We scale based on the maximum input & output coordinates, rather than the
  // input and output sizes, so that it's possible to reach the edge of the
  // output when up-scaling.  We also take care to round up or down correctly,
  // which is important when down-scaling.
  protocol::MouseEvent out_event(event);
  if (out_event.has_x()) {
    int x = out_event.x() * output_max_.width();
    x = (x + input_max_.width() / 2) / input_max_.width();
    out_event.set_x(std::max(0, std::min(output_max_.width(), x)));
  }
  if (out_event.has_y()) {
    int y = out_event.y() * output_max_.height();
    y = (y + input_max_.height() / 2) / input_max_.height();
    out_event.set_y(std::max(0, std::min(output_max_.height(), y)));
  }

  input_stub_->InjectMouseEvent(out_event);
}

void MouseInputFilter::set_input_size(const SkISize& size) {
  input_max_ = SkISize::Make(size.width() - 1, size.height() - 1);
}

void MouseInputFilter::set_output_size(const SkISize& size) {
  output_max_ = SkISize::Make(size.width() - 1, size.height() - 1);
}

}  // namespace remoting

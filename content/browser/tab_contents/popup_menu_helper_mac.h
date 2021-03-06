// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_BROWSER_TAB_CONTENTS_POPUP_MENU_HELPER_MAC_H_
#define CONTENT_BROWSER_TAB_CONTENTS_POPUP_MENU_HELPER_MAC_H_

#include <vector>

#include "base/compiler_specific.h"
#include "content/public/browser/notification_observer.h"
#include "content/public/browser/notification_registrar.h"
#include "ui/gfx/rect.h"

struct WebMenuItem;

namespace content {
class RenderViewHost;
class RenderViewHostImpl;
}

class PopupMenuHelper : public content::NotificationObserver {
 public:
  // Creates a PopupMenuHelper that will notify |render_view_host| when a user
  // selects or cancels the popup.
  explicit PopupMenuHelper(content::RenderViewHost* render_view_host);

  // Shows the popup menu and notifies the RenderViewHost of the selection/
  // cancel.
  // This call is blocking.
  void ShowPopupMenu(const gfx::Rect& bounds,
                     int item_height,
                     double item_font_size,
                     int selected_item,
                     const std::vector<WebMenuItem>& items,
                     bool right_aligned);

 private:
  // content::NotificationObserver implementation:
  virtual void Observe(int type,
                       const content::NotificationSource& source,
                       const content::NotificationDetails& details) OVERRIDE;

  content::NotificationRegistrar notification_registrar_;

  content::RenderViewHostImpl* render_view_host_;

  DISALLOW_COPY_AND_ASSIGN(PopupMenuHelper);
};

#endif  // CONTENT_BROWSER_TAB_CONTENTS_POPUP_MENU_HELPER_MAC_H_

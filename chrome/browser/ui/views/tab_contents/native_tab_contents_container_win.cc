// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/ui/views/tab_contents/native_tab_contents_container_win.h"

#include "chrome/browser/ui/view_ids.h"
#include "chrome/browser/ui/views/tab_contents/tab_contents_container.h"
#include "chrome/browser/ui/views/tab_contents/tab_contents_view_views.h"
#include "content/public/browser/render_widget_host_view.h"
#include "content/public/browser/web_contents.h"
#include "ui/base/accessibility/accessible_view_state.h"
#include "ui/views/focus/focus_manager.h"
#include "ui/views/focus/widget_focus_manager.h"

using content::RenderViewHost;
using content::WebContents;

////////////////////////////////////////////////////////////////////////////////
// NativeTabContentsContainerWin, public:

NativeTabContentsContainerWin::NativeTabContentsContainerWin(
    TabContentsContainer* container)
    : container_(container) {
  set_id(VIEW_ID_TAB_CONTAINER_FOCUS_VIEW);
}

NativeTabContentsContainerWin::~NativeTabContentsContainerWin() {
}

////////////////////////////////////////////////////////////////////////////////
// NativeTabContentsContainerWin, NativeTabContentsContainer overrides:

void NativeTabContentsContainerWin::AttachContents(WebContents* contents) {
  // We need to register the tab contents window with the BrowserContainer so
  // that the BrowserContainer is the focused view when the focus is on the
  // TabContents window (for the TabContents case).
  set_focus_view(this);

  Attach(contents->GetNativeView());
}

void NativeTabContentsContainerWin::DetachContents(WebContents* contents) {
  Detach();

  HWND container_hwnd = contents->GetNativeView();
  if (container_hwnd)
    ShowWindow(container_hwnd, SW_HIDE);
}

void NativeTabContentsContainerWin::SetFastResize(bool fast_resize) {
  set_fast_resize(fast_resize);
}

bool NativeTabContentsContainerWin::GetFastResize() const {
  return fast_resize();
}

bool NativeTabContentsContainerWin::FastResizeAtLastLayout() const {
  return fast_resize_at_last_layout();
}

void NativeTabContentsContainerWin::RenderViewHostChanged(
    RenderViewHost* old_host,
    RenderViewHost* new_host) {
  // If we are focused, we need to pass the focus to the new RenderViewHost.
  if (GetFocusManager()->GetFocusedView() == this)
    OnFocus();
}

views::View* NativeTabContentsContainerWin::GetView() {
  return this;
}

void NativeTabContentsContainerWin::WebContentsFocused(WebContents* contents) {
  views::FocusManager* focus_manager = GetFocusManager();
  if (!focus_manager) {
    NOTREACHED();
    return;
  }
  focus_manager->SetFocusedView(this);
}

////////////////////////////////////////////////////////////////////////////////
// NativeTabContentsContainerWin, views::View overrides:

bool NativeTabContentsContainerWin::SkipDefaultKeyEventProcessing(
    const views::KeyEvent& e) {
  // Don't look-up accelerators or tab-traversal if we are showing a non-crashed
  // TabContents.
  // We'll first give the page a chance to process the key events.  If it does
  // not process them, they'll be returned to us and we'll treat them as
  // accelerators then.
  return container_->web_contents() &&
         !container_->web_contents()->IsCrashed();
}

bool NativeTabContentsContainerWin::IsFocusable() const {
  // We need to be focusable when our contents is not a view hierarchy, as
  // clicking on the contents needs to focus us.
  return container_->web_contents() != NULL;
}

void NativeTabContentsContainerWin::OnFocus() {
  if (container_->web_contents())
    container_->web_contents()->Focus();
}

void NativeTabContentsContainerWin::RequestFocus() {
  // This is a hack to circumvent the fact that a the OnFocus() method is not
  // invoked when RequestFocus() is called on an already focused view.
  // The TabContentsContainer is the view focused when the TabContents has
  // focus.  When switching between from one tab that has focus to another tab
  // that should also have focus, RequestFocus() is invoked one the
  // TabContentsContainer.  In order to make sure OnFocus() is invoked we need
  // to clear the focus before hands.
  {
    // Disable notifications.  Clear focus will assign the focus to the main
    // browser window.  Because this change of focus was not user requested,
    // don't send it to listeners.
    views::AutoNativeNotificationDisabler local_notification_disabler;
    views::FocusManager* focus_manager = GetFocusManager();
    if (focus_manager)  // NULL in unittests when using TabContentsViewWin.
      focus_manager->ClearFocus();
  }
  View::RequestFocus();
}

void NativeTabContentsContainerWin::AboutToRequestFocusFromTabTraversal(
    bool reverse) {
  container_->web_contents()->FocusThroughTabTraversal(reverse);
}

void NativeTabContentsContainerWin::GetAccessibleState(
    ui::AccessibleViewState* state) {
  state->role = ui::AccessibilityTypes::ROLE_GROUPING;
}

gfx::NativeViewAccessible
    NativeTabContentsContainerWin::GetNativeViewAccessible() {
  WebContents* web_contents = container_->web_contents();
  if (web_contents) {
    content::RenderWidgetHostView* host_view =
        web_contents->GetRenderWidgetHostView();
    if (host_view)
      return host_view->GetNativeViewAccessible();
  }

  return View::GetNativeViewAccessible();
}

////////////////////////////////////////////////////////////////////////////////
// NativeTabContentsContainer, public:

// static
NativeTabContentsContainer* NativeTabContentsContainer::CreateNativeContainer(
    TabContentsContainer* container) {
  return new NativeTabContentsContainerWin(container);
}

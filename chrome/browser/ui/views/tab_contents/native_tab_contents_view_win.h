// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_UI_VIEWS_TAB_CONTENTS_NATIVE_TAB_CONTENTS_VIEW_WIN_H_
#define CHROME_BROWSER_UI_VIEWS_TAB_CONTENTS_NATIVE_TAB_CONTENTS_VIEW_WIN_H_
#pragma once

#include "chrome/browser/ui/views/tab_contents/native_tab_contents_view.h"
#include "ui/views/widget/native_widget_win.h"

class WebDragBookmarkHandlerWin;
class WebDragDest;
class WebContentsDragWin;

namespace content {
class RenderWidgetHostView;
class WebContents;
}

class NativeTabContentsViewWin : public views::NativeWidgetWin,
                                 public NativeTabContentsView {
 public:
  explicit NativeTabContentsViewWin(
      internal::NativeTabContentsViewDelegate* delegate);
  virtual ~NativeTabContentsViewWin();

  WebDragDest* drag_dest() const { return drag_dest_.get(); }

  content::WebContents* GetWebContents() const;

 private:
  // Overridden from NativeTabContentsView:
  virtual void InitNativeTabContentsView() OVERRIDE;
  virtual content::RenderWidgetHostView* CreateRenderWidgetHostView(
      content::RenderWidgetHost* render_widget_host) OVERRIDE;
  virtual gfx::NativeWindow GetTopLevelNativeWindow() const OVERRIDE;
  virtual void SetPageTitle(const string16& title) OVERRIDE;
  virtual void StartDragging(const WebDropData& drop_data,
                             WebKit::WebDragOperationsMask ops,
                             const SkBitmap& image,
                             const gfx::Point& image_offset) OVERRIDE;
  virtual void CancelDrag() OVERRIDE;
  virtual bool IsDoingDrag() const OVERRIDE;
  virtual void SetDragCursor(WebKit::WebDragOperation operation) OVERRIDE;
  virtual views::NativeWidget* AsNativeWidget() OVERRIDE;

  // Overridden from views::NativeWidgetWin:
  virtual void OnDestroy() OVERRIDE;
  virtual void OnHScroll(int scroll_type,
                         short position,
                         HWND scrollbar) OVERRIDE;
  virtual LRESULT OnMouseRange(UINT msg,
                               WPARAM w_param,
                               LPARAM l_param) OVERRIDE;
  virtual LRESULT OnReflectedMessage(UINT msg,
                                     WPARAM w_param,
                                     LPARAM l_param) OVERRIDE;
  virtual void OnVScroll(int scroll_type,
                         short position,
                         HWND scrollbar) OVERRIDE;
  virtual void OnWindowPosChanged(WINDOWPOS* window_pos) OVERRIDE;
  virtual void OnSize(UINT param, const WTL::CSize& size) OVERRIDE;
  virtual LRESULT OnNCCalcSize(BOOL w_param, LPARAM l_param) OVERRIDE;
  virtual void OnNCPaint(HRGN rgn) OVERRIDE;
  virtual LRESULT OnNCHitTest(const CPoint& point) OVERRIDE;

  // Backend for all scroll messages, the |message| parameter indicates which
  // one it is.
  void ScrollCommon(UINT message, int scroll_type, short position,
                    HWND scrollbar);

  void EndDragging();

  internal::NativeTabContentsViewDelegate* delegate_;

  scoped_ptr<WebDragBookmarkHandlerWin> bookmark_handler_;
  // A drop target object that handles drags over this TabContents.
  scoped_refptr<WebDragDest> drag_dest_;

  // Used to handle the drag-and-drop.
  scoped_refptr<WebContentsDragWin> drag_handler_;

  DISALLOW_COPY_AND_ASSIGN(NativeTabContentsViewWin);
};

#endif  // CHROME_BROWSER_UI_VIEWS_TAB_CONTENTS_NATIVE_TAB_CONTENTS_VIEW_WIN_H_

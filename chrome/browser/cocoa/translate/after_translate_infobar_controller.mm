// Copyright (c) 2010 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/cocoa/translate/after_translate_infobar_controller.h"
#include "base/sys_string_conversions.h"

using TranslateInfoBarUtilities::MoveControl;

@implementation AfterTranslateInfobarController

- (void)loadLabelText {
  std::vector<string16> strings;
  TranslateInfoBarDelegate::GetAfterTranslateStrings(
      &strings, &swappedLanugageButtons_);
  DCHECK(strings.size() == 3U);
  NSString* string1 = base::SysUTF16ToNSString(strings[0]);
  NSString* string2 = base::SysUTF16ToNSString(strings[1]);
  NSString* string3 = base::SysUTF16ToNSString(strings[2]);

  [label1_ setStringValue:string1];
  [label2_ setStringValue:string2];
  [label3_ setStringValue:string3];
}

- (void)layout {
  [self removeOkCancelButtons];
  [optionsPopUp_ setHidden:NO];
  NSView* firstPopup = fromLanguagePopUp_;
  NSView* lastPopup = toLanguagePopUp_;
  if (swappedLanugageButtons_) {
    firstPopup = toLanguagePopUp_;
    lastPopup = fromLanguagePopUp_;
  }
  NSView* lastControl = lastPopup;

  MoveControl(label1_, firstPopup, 0, true);
  MoveControl(firstPopup, label2_, 0, true);
  MoveControl(label2_, lastPopup, 0, true);
  MoveControl(lastPopup, label3_, 0, true);
  lastControl = label3_;

  MoveControl(lastControl, showOriginalButton_, spaceBetweenControls_ * 2,
      true);
}

- (NSArray*)visibleControls {
  return [NSArray arrayWithObjects:label1_.get(), fromLanguagePopUp_.get(),
      label2_.get(), toLanguagePopUp_.get(), label3_.get(),
      showOriginalButton_.get(), nil];
}

- (bool)verifyLayout {
  if ([optionsPopUp_ isHidden])
    return false;
  return [super verifyLayout];
}

@end

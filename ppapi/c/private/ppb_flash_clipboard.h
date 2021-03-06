/* Copyright (c) 2012 The Chromium Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

/* From private/ppb_flash_clipboard.idl modified Thu Feb 23 23:15:40 2012. */

#ifndef PPAPI_C_PRIVATE_PPB_FLASH_CLIPBOARD_H_
#define PPAPI_C_PRIVATE_PPB_FLASH_CLIPBOARD_H_

#include "ppapi/c/pp_bool.h"
#include "ppapi/c/pp_instance.h"
#include "ppapi/c/pp_macros.h"
#include "ppapi/c/pp_stdint.h"
#include "ppapi/c/pp_var.h"

#define PPB_FLASH_CLIPBOARD_INTERFACE_3_0 "PPB_Flash_Clipboard;3.0"
#define PPB_FLASH_CLIPBOARD_INTERFACE_4_0 "PPB_Flash_Clipboard;4.0"
#define PPB_FLASH_CLIPBOARD_INTERFACE PPB_FLASH_CLIPBOARD_INTERFACE_4_0

/**
 * @file
 * This file defines the private <code>PPB_Flash_Clipboard</code> API used by
 * Pepper Flash for reading and writing to the clipboard.
 */


/**
 * The old version string for this interface, equivalent to version 3.0.
 * TODO(viettrungluu): Remove this when enough time has passed. crbug.com/104184
 */
#define PPB_FLASH_CLIPBOARD_INTERFACE_3_LEGACY "PPB_Flash_Clipboard;3"

/**
 * @addtogroup Enums
 * @{
 */
/**
 * This enumeration contains the types of clipboards that can be accessed.
 * These types correspond to clipboard types in WebKit.
 */
typedef enum {
  /** The standard clipboard. */
  PP_FLASH_CLIPBOARD_TYPE_STANDARD = 0,
  /** The selection clipboard (e.g., on Linux). */
  PP_FLASH_CLIPBOARD_TYPE_SELECTION = 1
} PP_Flash_Clipboard_Type;
PP_COMPILE_ASSERT_SIZE_IN_BYTES(PP_Flash_Clipboard_Type, 4);

/**
 * This enumeration contains the supported clipboard data formats.
 */
typedef enum {
  /** Indicates an invalid or unsupported clipboard data format. */
  PP_FLASH_CLIPBOARD_FORMAT_INVALID = 0,
  /** Indicates plain text clipboard data. */
  PP_FLASH_CLIPBOARD_FORMAT_PLAINTEXT = 1,
  /** Indicates HTML clipboard data. */
  PP_FLASH_CLIPBOARD_FORMAT_HTML = 2
} PP_Flash_Clipboard_Format;
PP_COMPILE_ASSERT_SIZE_IN_BYTES(PP_Flash_Clipboard_Format, 4);
/**
 * @}
 */

/**
 * @addtogroup Interfaces
 * @{
 */
/**
 * The <code>PPB_Flash_Clipboard</code> interface contains pointers to functions
 * used by Pepper Flash to access the clipboard.
 *
 */
struct PPB_Flash_Clipboard_4_0 {
  /**
   * Checks whether a given data format is available from the given clipboard.
   * Returns true if the given format is available from the given clipboard.
   */
  PP_Bool (*IsFormatAvailable)(PP_Instance instance_id,
                               PP_Flash_Clipboard_Type clipboard_type,
                               PP_Flash_Clipboard_Format format);
  /**
   * Reads data in the given <code>format</code> from the clipboard. An
   * undefined <code>PP_Var</code> is returned if there is an error in reading
   * the clipboard data and a null <code>PP_Var</code> is returned if there is
   * no data of the specified <code>format</code> to read.
   */
  struct PP_Var (*ReadData)(PP_Instance instance_id,
                            PP_Flash_Clipboard_Type clipboard_type,
                            PP_Flash_Clipboard_Format format);
  /**
   * Writes the given array of data items to the clipboard. All existing
   * clipboard data in any format is erased before writing this data. Thus,
   * passing an array of size 0 has the effect of clearing the clipboard without
   * writing any data. Each data item in the array should have a different
   * <code>PP_Flash_Clipboard_Format</code>. If multiple data items have the
   * same format, only the last item with that format will be written.
   * If there is an error writing any of the items in the array to the
   * clipboard, none will be written and an error code is returned.
   * The error code will be <code>PP_ERROR_NOSPACE</code> if the value is
   * too large to be written, <code>PP_ERROR_BADARGUMENT</code> if a PP_Var
   * cannot be converted into the format supplied or <code>PP_FAILED</code>
   * if the format is not supported.
   */
  int32_t (*WriteData)(PP_Instance instance_id,
                       PP_Flash_Clipboard_Type clipboard_type,
                       uint32_t data_item_count,
                       const PP_Flash_Clipboard_Format formats[],
                       const struct PP_Var data_items[]);
};

typedef struct PPB_Flash_Clipboard_4_0 PPB_Flash_Clipboard;

struct PPB_Flash_Clipboard_3_0 {
  PP_Bool (*IsFormatAvailable)(PP_Instance instance_id,
                               PP_Flash_Clipboard_Type clipboard_type,
                               PP_Flash_Clipboard_Format format);
  struct PP_Var (*ReadPlainText)(PP_Instance instance_id,
                                 PP_Flash_Clipboard_Type clipboard_type);
  int32_t (*WritePlainText)(PP_Instance instance_id,
                            PP_Flash_Clipboard_Type clipboard_type,
                            struct PP_Var text);
};
/**
 * @}
 */

#endif  /* PPAPI_C_PRIVATE_PPB_FLASH_CLIPBOARD_H_ */


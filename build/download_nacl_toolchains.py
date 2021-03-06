#!/usr/bin/env python
# Copyright (c) 2012 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Shim to run nacl toolchain download script only if there is a nacl dir."""

import os
import sys


def Main(args):
  # Exit early if disable_nacl=1.
  if 'disable_nacl=1' in os.environ.get('GYP_DEFINES', ''):
    return 0
  script_dir = os.path.dirname(os.path.abspath(__file__))
  src_dir = os.path.dirname(script_dir)
  nacl_dir = os.path.join(src_dir, 'native_client')
  nacl_build_dir = os.path.join(nacl_dir, 'build')
  download_script = os.path.join(nacl_build_dir, 'download_toolchains.py')
  if not os.path.exists(download_script):
    print "Can't find '%s'" % download_script
    print 'Presumably you are intentionally building without NativeClient.'
    print 'Skipping NativeClient toolchain download.'
    sys.exit(0)
  sys.path.insert(0, nacl_build_dir)
  import download_toolchains

  # TODO (robertm): Finish getting PNaCl ready for prime time.
  # BUG:
  # We remove this --optional-pnacl argument, and instead replace it with
  # --no-pnacl for most cases.  However, if the bot name is the pnacl_sdk
  # bot then we will go ahead and download it.  This prevents increasing the
  # gclient sync time for developers, or standard Chrome bots.
  if '--optional-pnacl' in args:
    args.remove('--optional-pnacl')
    buildbot_name = os.environ.get('BUILDBOT_BUILDERNAME', '')
    if buildbot_name.find('pnacl') >= 0 and  buildbot_name.find('sdk') >= 0:
      print '\n*** DOWNLOADING PNACL TOOLCHAIN ***\n'
    else:
      args.append('--no-pnacl')

  download_toolchains.Main(args)
  return 0


if __name__ == '__main__':
  sys.exit(Main(sys.argv[1:]))


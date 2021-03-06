# Copyright (c) 2012 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import json

import pyauto_functional  # must be imported before pyauto
import pyauto


class PyAutoEventsTest(pyauto.PyUITest):

  def testBasicEvents(self):
    """Basic test for the event queue."""
    url = self.GetHttpURLForDataPath('apptest', 'basic.html')
    driver = self.NewWebDriver()
    event_id = self.AddDomRaisedEventObserver();
    self.NavigateToURL(url)
    self._ExpectEvent(event_id, 'init')
    self._ExpectEvent(event_id, 'login ready')
    driver.find_element_by_id('login').click()
    self._ExpectEvent(event_id, 'login start')
    self._ExpectEvent(event_id, 'login done')

  def _ExpectEvent(self, event_id, event_name):
    # TODO(craigdh): Temporary hack to ignore unexpected events generated by
    # chromedriver's use of DomAutomationController. The upcoming revision to
    # RaisedEvents will fix this. Note this isn't polling, just ignoring
    # chromedriver events.
    while True:
      event = json.loads(self.GetNextEvent(event_id).get('name'))
      if event.find('{') == -1:
        break
    self.assertEqual(event, event_name)


if __name__ == '__main__':
  pyauto_functional.Main()

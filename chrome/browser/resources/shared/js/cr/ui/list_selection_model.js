// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

cr.define('cr.ui', function() {
  const Event = cr.Event;
  const EventTarget = cr.EventTarget;

  /**
   * Creates a new selection model that is to be used with lists.
   *
   * @param {number=} opt_length The number items in the selection.
   *
   * @constructor
   * @extends {!cr.EventTarget}
   */
  function ListSelectionModel(opt_length) {
    this.length_ = opt_length || 0;
    // Even though selectedIndexes_ is really a map we use an array here to get
    // iteration in the order of the indexes.
    this.selectedIndexes_ = [];
  }

  ListSelectionModel.prototype = {
    __proto__: EventTarget.prototype,

    /**
     * The number of items in the model.
     * @type {number}
     */
    get length() {
      return this.length_;
    },

    /**
     * The selected indexes.
     * Setter also changes lead and anchor indexes if value list is nonempty.
     * @type {!Array}
     */
    get selectedIndexes() {
      return Object.keys(this.selectedIndexes_).map(Number);
    },
    set selectedIndexes(selectedIndexes) {
      this.beginChange();
      var unselected = {};
      for (var index in this.selectedIndexes_) {
        unselected[index] = true;
      }

      for (var i = 0; i < selectedIndexes.length; i++) {
        var index = selectedIndexes[i];
        if (index in this.selectedIndexes_) {
          delete unselected[index];
        } else {
          this.selectedIndexes_[index] = true;
          this.changedIndexes_[index] = true;
        }
      }

      for (var index in unselected) {
        delete this.selectedIndexes_[index];
        this.changedIndexes_[index] = false;
      }

      if (selectedIndexes.length) {
        this.leadIndex = this.anchorIndex = selectedIndexes[0];
      } else {
        this.leadIndex = this.anchorIndex = -1;
      }
      this.endChange();
    },

    /**
     * Convenience getter which returns the first selected index.
     * Setter also changes lead and anchor indexes if value is nonnegative.
     * @type {number}
     */
    get selectedIndex() {
      for (var i in this.selectedIndexes_) {
        return Number(i);
      }
      return -1;
    },
    set selectedIndex(selectedIndex) {
      this.selectedIndexes = selectedIndex != -1 ? [selectedIndex] : [];
    },

    /**
     * Selects a range of indexes, starting with {@code start} and ends with
     * {@code end}.
     * @param {number} start The first index to select.
     * @param {number} end The last index to select.
     */
    selectRange: function(start, end) {
      // Swap if starts comes after end.
      if (start > end) {
        var tmp = start;
        start = end;
        end = tmp;
      }

      this.beginChange();

      for (var index = start; index != end; index++) {
        this.setIndexSelected(index, true);
      }
      this.setIndexSelected(end, true);

      this.endChange();
    },

    /**
     * Selects all indexes.
     */
    selectAll: function() {
      this.selectRange(0, this.length - 1);
    },

    /**
     * Clears the selection
     */
    clear: function() {
      this.beginChange();
      this.length_ = 0;
      this.anchorIndex = this.leadIndex = -1;
      this.unselectAll();
      this.endChange();
    },

    /**
     * Unselects all selected items.
     */
    unselectAll: function() {
      this.beginChange();
      for (var i in this.selectedIndexes_) {
        this.setIndexSelected(i, false);
      }
      this.endChange();
    },

    /**
     * Sets the selected state for an index.
     * @param {number} index The index to set the selected state for.
     * @param {boolean} b Whether to select the index or not.
     */
    setIndexSelected: function(index, b) {
      var oldSelected = index in this.selectedIndexes_;
      if (oldSelected == b)
        return;

      if (b)
        this.selectedIndexes_[index] = true;
      else
        delete this.selectedIndexes_[index];

      this.beginChange();

      this.changedIndexes_[index] = b;

      // End change dispatches an event which in turn may update the view.
      this.endChange();
    },

    /**
     * Whether a given index is selected or not.
     * @param {number} index The index to check.
     * @return {boolean} Whether an index is selected.
     */
    getIndexSelected: function(index) {
      return index in this.selectedIndexes_;
    },

    /**
     * This is used to begin batching changes. Call {@code endChange} when you
     * are done making changes.
     */
    beginChange: function() {
      if (!this.changeCount_) {
        this.changeCount_ = 0;
        this.changedIndexes_ = {};
      }
      this.changeCount_++;
    },

    /**
     * Call this after changes are done and it will dispatch a change event if
     * any changes were actually done.
     */
    endChange: function() {
      this.changeCount_--;
      if (!this.changeCount_) {
        // Calls delayed |dispatchPropertyChange|s, only when |leadIndex| or
        // |anchorIndex| has been actually changed in the batch.
        if (this.oldLeadIndex_ != null) {
          cr.dispatchPropertyChange(this, 'leadIndex',
                                    this.leadIndex_, this.oldLeadIndex_);
          this.oldLeadIndex_ = null;
        }

        if (this.oldAnchorIndex_ != null) {
          cr.dispatchPropertyChange(this, 'anchorIndex',
                                    this.anchorIndex_, this.oldAnchorIndex_);
          this.oldAnchorIndex_ = null;
        }

        var indexes = Object.keys(this.changedIndexes_);
        if (indexes.length) {
          var e = new Event('change');
          e.changes = indexes.map(function(index) {
            return {
              index: index,
              selected: this.changedIndexes_[index]
            };
          }, this);
          this.dispatchEvent(e);
        }
        this.changedIndexes_ = {};
      }
    },

    leadIndex_: -1,
    oldLeadIndex_: null,

    /**
     * The leadIndex is used with multiple selection and it is the index that
     * the user is moving using the arrow keys.
     * @type {number}
     */
    get leadIndex() {
      return this.leadIndex_;
    },
    set leadIndex(leadIndex) {
      var li = Math.max(-1, Math.min(this.length_ - 1, leadIndex));
      if (li != this.leadIndex_) {
        var oldLeadIndex = this.leadIndex_;
        this.leadIndex_ = li;

        // Delays the call of dispatchPropertyChange if batch is running.
        if (this.changeCount_) {
          if (this.oldLeadIndex_ == null)
            this.oldLeadIndex_ = oldLeadIndex;
        } else {
          cr.dispatchPropertyChange(this, 'leadIndex', li, oldLeadIndex);
        }
      }
    },

    anchorIndex_: -1,
    oldAnchorIndex_: null,

    /**
     * The anchorIndex is used with multiple selection.
     * @type {number}
     */
    get anchorIndex() {
      return this.anchorIndex_;
    },
    set anchorIndex(anchorIndex) {
      var ai = Math.max(-1, Math.min(this.length_ - 1, anchorIndex));
      if (ai != this.anchorIndex_) {
        var oldAnchorIndex = this.anchorIndex_;
        this.anchorIndex_ = ai;

        // Delays the call of dispatchPropertyChange if batch is running.
        if (this.changeCount_) {
          if (this.oldAnchorIndex_ == null)
            this.oldAnchorIndex_ = oldAnchorIndex;
        } else {
          cr.dispatchPropertyChange(this, 'anchorIndex', ai, oldAnchorIndex);
        }
      }
    },

    /**
     * Whether the selection model supports multiple selected items.
     * @type {boolean}
     */
    get multiple() {
      return true;
    },

    /**
     * Adjusts the selection after reordering of items in the table.
     * @param {!Array.<number>} permutation The reordering permutation.
     */
    adjustToReordering: function(permutation) {
      var oldLeadIndex = this.leadIndex;

      var oldSelectedIndexes = this.selectedIndexes;
      this.selectedIndexes = oldSelectedIndexes.map(function(oldIndex) {
        return permutation[oldIndex];
      }).filter(function(index) {
        return index != -1;
      });

      if (oldLeadIndex != -1)
        this.leadIndex = permutation[oldLeadIndex];
    },

    /**
     * Adjusts selection model length.
     * @param {number} length New selection model length.
     */
    adjustLength: function(length) {
      this.length_ = length;
    }
  };

  return {
    ListSelectionModel: ListSelectionModel
  };
});

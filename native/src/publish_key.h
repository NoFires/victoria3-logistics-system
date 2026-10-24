// Publication dedupe key for the logistics GUI variables.
//
// The variables are a pure function of the published document plus the
// scope-bearing status fields below. Republishing an identical state rewrote the
// same strings every frame (the hidden debug panel used to publish a null
// document ~65 times/second), so the key lets the caller skip that work.
#pragma once
#include <cstdint>

namespace v3 {

struct PublishedKey {
  const void* document = nullptr;
  uint32_t owner = UINT32_MAX;
  uint64_t world = 0, revision = 0;
  bool panelUnavailable = true, goodsUnavailable = true, worldRetired = false;
  bool valid = false;  // false until something has actually been published

  // Two keys are "the same publication" only when both have been published and
  // every field that can change the emitted strings matches.
  bool operator==(const PublishedKey& other) const {
    return valid && other.valid && document == other.document && owner == other.owner &&
           world == other.world && revision == other.revision &&
           panelUnavailable == other.panelUnavailable &&
           goodsUnavailable == other.goodsUnavailable && worldRetired == other.worldRetired;
  }
  bool operator!=(const PublishedKey& other) const { return !(*this == other); }
};

}  // namespace v3

// Regression for the publication dedupe key (see publish_key.h).
//
// Repro of the defect this guards: the hidden debug panel published a null
// document every frame, so the same GUI variables were rewritten ~65 times per
// second. The key must treat that repeat as "same publication" while still
// publishing whenever anything that changes the emitted strings changes.
#include <cstdio>

#include "publish_key.h"

namespace {

int g_failures = 0;

void check(bool condition, const char* what) {
  if (!condition) {
    std::printf("  FAIL %s\n", what);
    ++g_failures;
  }
}

v3::PublishedKey make(const void* document, uint32_t owner, uint64_t world, uint64_t revision,
                      bool panelUnavailable, bool goodsUnavailable, bool worldRetired) {
  v3::PublishedKey key;
  key.document = document;
  key.owner = owner;
  key.world = world;
  key.revision = revision;
  key.panelUnavailable = panelUnavailable;
  key.goodsUnavailable = goodsUnavailable;
  key.worldRetired = worldRetired;
  key.valid = true;
  return key;
}

}  // namespace

int main() {
  // Unpublished keys never match, so the first publication always happens.
  v3::PublishedKey a, b;
  check(a != b, "two never-published keys must differ");

  static int documentA, documentB;
  const v3::PublishedKey base = make(&documentA, 10, 1, 7, false, false, false);

  // The repeated-null publication the defect produced must be skipped.
  const v3::PublishedKey repeatNull = make(nullptr, 0, 1, 7, false, false, false);
  const v3::PublishedKey repeatNullAgain = make(nullptr, 0, 1, 7, false, false, false);
  check(repeatNull == repeatNullAgain, "identical null publications must compare equal");

  // ... but the first one after a real document must still publish.
  check(make(nullptr, 0, 1, 7, false, false, false) != base,
        "null after a document must publish");

  // Same object pointer, same status: skip.
  check(base == make(&documentA, 10, 1, 7, false, false, false),
        "identical re-publication must compare equal");

  // Any scope-bearing field changing must publish.
  check(base != make(&documentB, 10, 1, 7, false, false, false), "document change must publish");
  check(base != make(&documentA, 20, 1, 7, false, false, false), "owner change must publish");
  check(base != make(&documentA, 10, 2, 7, false, false, false), "world change must publish");
  check(base != make(&documentA, 10, 1, 8, false, false, false), "revision change must publish");
  check(base != make(&documentA, 10, 1, 7, true, false, false),
        "panel availability change must publish");
  check(base != make(&documentA, 10, 1, 7, false, true, false),
        "goods availability change must publish");
  check(base != make(&documentA, 10, 1, 7, false, false, true),
        "world-retired change must publish");

  if (g_failures) {
    std::printf("FAIL publish key dedupe (%d checks)\n", g_failures);
    return 1;
  }
  std::printf("PASS publication dedupe key: identical publications skipped, every scope-bearing "
              "field still publishes\n");
  return 0;
}

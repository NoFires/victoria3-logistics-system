"""Executable Python reference for the request/cache policy, not a game plugin.

The native implementation must keep all graphics calls on verified render paths.
"""
from dataclasses import dataclass
from time import perf_counter


@dataclass(frozen=True)
class Ribbon:
    points: tuple
    style: str


class RenderModel:
    def __init__(self, capacity=8192):
        self.capacity = capacity
        self.cache = {}
        self.next_slot = 0
        self.epoch = 0
        self.signature = ()
        self.todo = []
        self.active = []
        self.uploads = 0
        self.skipped = 0

    def request(self, ribbons):
        signature = tuple(dict.fromkeys(ribbons))
        if signature == self.signature:
            self.skipped += 1
            return False
        self.signature = signature
        self.epoch += 1
        self.todo = list(signature)
        self.active = []
        for ribbon in signature:
            if ribbon in self.cache:
                slot, _, valid = self.cache[ribbon]
                self.cache[ribbon] = slot, self.epoch, valid
        return True

    def pump(self, upload, max_uploads=16, seconds=0.002, clock=perf_counter):
        started, attempts = clock(), 0
        while self.todo and attempts < max_uploads and clock() - started < seconds:
            ribbon = self.todo.pop(0)
            if len(ribbon.points) < 2 or len(set(ribbon.points)) < 2:
                continue
            if ribbon not in self.cache:
                if len(self.cache) >= self.capacity:
                    victims = [(entry[1], old) for old, entry in self.cache.items()
                               if old.style == ribbon.style and entry[1] < self.epoch]
                    if not victims:
                        continue
                    old = min(victims, key=lambda item: item[0])[1]
                    slot, _, _ = self.cache.pop(old)
                else:
                    slot = self.next_slot
                    self.next_slot += 1
                attempts += 1
                self.uploads += 1
                valid = bool(upload(slot, ribbon))
                self.cache[ribbon] = slot, self.epoch, valid
            slot, _, valid = self.cache[ribbon]
            if valid:
                self.active.append(slot)
        return not self.todo


class SettleGate:
    def __init__(self):
        self.first = self.last = None

    def change(self, now):
        if self.first is None:
            self.first = now
        self.last = now

    def ready(self, now):
        return self.first is not None and (now - self.last >= 0.100 or now - self.first >= 0.250)

    def consume(self):
        self.first = self.last = None

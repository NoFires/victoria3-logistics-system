#pragma once
#include <windows.h>

namespace v3 {

// Win32 critical section. Avoids a libwinpthread runtime dependency for the
// deployed DLL. Worker mailboxes hold it only to exchange owned requests/results;
// graph searches and native rendering never run under this lock.
class Mutex {
 public:
  Mutex() { InitializeCriticalSection(&m_cs); }
  ~Mutex() { DeleteCriticalSection(&m_cs); }
  Mutex(const Mutex&) = delete;
  Mutex& operator=(const Mutex&) = delete;
  void Lock() { EnterCriticalSection(&m_cs); }
  void Unlock() { LeaveCriticalSection(&m_cs); }
  CRITICAL_SECTION* Raw() { return &m_cs; }

 private:
  CRITICAL_SECTION m_cs;
};

class LockGuard {
 public:
  explicit LockGuard(Mutex& m) : m_mutex(m) { m_mutex.Lock(); }
  ~LockGuard() { m_mutex.Unlock(); }
  LockGuard(const LockGuard&) = delete;
  LockGuard& operator=(const LockGuard&) = delete;

 private:
  Mutex& m_mutex;
};

}  // namespace v3

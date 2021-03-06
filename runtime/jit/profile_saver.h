/*
 * Copyright (C) 2015 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef ART_RUNTIME_JIT_PROFILE_SAVER_H_
#define ART_RUNTIME_JIT_PROFILE_SAVER_H_

#include "base/mutex.h"
#include "jit_code_cache.h"
#include "offline_profiling_info.h"
#include "safe_map.h"

namespace art {

class ProfileSaver {
 public:
  // Starts the profile saver thread if not already started.
  // If the saver is already running it adds (output_filename, code_paths) to its tracked locations.
  static void Start(const std::string& output_filename,
                    jit::JitCodeCache* jit_code_cache,
                    const std::vector<std::string>& code_paths,
                    const std::string& foreign_dex_profile_path,
                    const std::string& app_data_dir)
      REQUIRES(!Locks::profiler_lock_, !wait_lock_);

  // Stops the profile saver thread.
  // NO_THREAD_SAFETY_ANALYSIS for static function calling into member function with excludes lock.
  static void Stop()
      REQUIRES(!Locks::profiler_lock_, !wait_lock_)
      NO_THREAD_SAFETY_ANALYSIS;

  // Returns true if the profile saver is started.
  static bool IsStarted() REQUIRES(!Locks::profiler_lock_);

  static void NotifyDexUse(const std::string& dex_location);

 private:
  ProfileSaver(const std::string& output_filename,
               jit::JitCodeCache* jit_code_cache,
               const std::vector<std::string>& code_paths,
               const std::string& foreign_dex_profile_path,
               const std::string& app_data_dir);

  // NO_THREAD_SAFETY_ANALYSIS for static function calling into member function with excludes lock.
  static void* RunProfileSaverThread(void* arg)
      REQUIRES(!Locks::profiler_lock_, !wait_lock_)
      NO_THREAD_SAFETY_ANALYSIS;

  // The run loop for the saver.
  void Run() REQUIRES(!Locks::profiler_lock_, !wait_lock_);
  // Processes the existing profiling info from the jit code cache and returns
  // true if it needed to be saved to disk.
  bool ProcessProfilingInfo();
  // Returns true if the saver is shutting down (ProfileSaver::Stop() has been called).
  bool ShuttingDown(Thread* self) REQUIRES(!Locks::profiler_lock_);

  void AddTrackedLocations(const std::string& output_filename,
                           const std::vector<std::string>& code_paths)
      REQUIRES(Locks::profiler_lock_);

  static void MaybeRecordDexUseInternal(
      const std::string& dex_location,
      const std::set<std::string>& tracked_locations,
      const std::string& foreign_dex_profile_path,
      const std::string& app_data_dir);

  // The only instance of the saver.
  static ProfileSaver* instance_ GUARDED_BY(Locks::profiler_lock_);
  // Profile saver thread.
  static pthread_t profiler_pthread_ GUARDED_BY(Locks::profiler_lock_);

  jit::JitCodeCache* jit_code_cache_;
  SafeMap<std::string, std::set<std::string>> tracked_dex_base_locations_
      GUARDED_BY(Locks::profiler_lock_);
  std::string foreign_dex_profile_path_;
  std::string app_data_dir_;
  uint64_t code_cache_last_update_time_ns_;
  bool shutting_down_ GUARDED_BY(Locks::profiler_lock_);
  bool first_profile_ = true;

  // Save period condition support.
  Mutex wait_lock_ DEFAULT_MUTEX_ACQUIRED_AFTER;
  ConditionVariable period_condition_ GUARDED_BY(wait_lock_);

  DISALLOW_COPY_AND_ASSIGN(ProfileSaver);
};

}  // namespace art

#endif  // ART_RUNTIME_JIT_PROFILE_SAVER_H_

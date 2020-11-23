// Copyright (C) 2020 The Android Open Source Project
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#pragma once

#include <unistd.h>

#include <chrono>
#include <cstring>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

#include <android-base/unique_fd.h>

namespace android {
namespace snapshot {

static constexpr uint32_t PACKET_SIZE = 512;

static constexpr char kSnapuserdSocket[] = "snapuserd";

static constexpr char kSnapuserdFirstStagePidVar[] = "FIRST_STAGE_SNAPUSERD_PID";

// Ensure that the second-stage daemon for snapuserd is running.
bool EnsureSnapuserdStarted();

class SnapuserdClient {
  private:
    android::base::unique_fd sockfd_;

    bool Sendmsg(const std::string& msg);
    std::string Receivemsg();

    bool ValidateConnection();

  public:
    explicit SnapuserdClient(android::base::unique_fd&& sockfd);

    static std::unique_ptr<SnapuserdClient> Connect(const std::string& socket_name,
                                                    std::chrono::milliseconds timeout_ms);

    bool StopSnapuserd();

    // Initializing a snapuserd handler is a three-step process:
    //
    //  1. Client invokes InitDmUserCow. This creates the snapuserd handler and validates the
    //     COW. The number of sectors required for the dm-user target is returned.
    //  2. Client creates the device-mapper device with the dm-user target.
    //  3. Client calls AttachControlDevice.
    //
    // The misc_name must be the "misc_name" given to dm-user in step 2.
    //
    uint64_t InitDmUserCow(const std::string& misc_name, const std::string& cow_device,
                           const std::string& backing_device);
    bool AttachDmUser(const std::string& misc_name);

    // Wait for snapuserd to disassociate with a dm-user control device. This
    // must ONLY be called if the control device has already been deleted.
    bool WaitForDeviceDelete(const std::string& control_device);

    // Detach snapuserd. This shuts down the listener socket, and will cause
    // snapuserd to gracefully exit once all handler threads have terminated.
    // This should only be used on first-stage instances of snapuserd.
    bool DetachSnapuserd();
};

}  // namespace snapshot
}  // namespace android

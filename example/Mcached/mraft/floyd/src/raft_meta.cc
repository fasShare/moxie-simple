// Copyright (c) 2015-present, Qihoo, Inc.  All rights reserved.
// This source code is licensed under the BSD-style license found in the
// LICENSE file in the root directory of this source tree. An additional grant
// of patent rights can be found in the PATENTS file in the same directory.

#include "floyd/src/raft_meta.h"

#include <stdlib.h>

#include "rocksdb/status.h"

#include "floyd/src/logger.h"
#include "floyd/src/floyd.pb.h"
#include "slash/include/env.h"
#include "slash/include/xdebug.h"

namespace floyd {

static const std::string kCurrentTerm = "CURRENTTERM";
static const std::string kVoteForIp = "VOTEFORIP";
static const std::string kVoteForPort = "VOTEFORPORT";
static const std::string kCommitIndex = "COMMITINDEX";
static const std::string kLastApplied = "APPLYINDEX";
/*
 * fencing token is not part of raft, fencing token is used for implementing distributed lock
 */
static const std::string kFencingToken = "FENCINGTOKEN";

RaftMeta::RaftMeta(Logger* info_log)
  : db_(),
    info_log_(info_log) {
}

RaftMeta::~RaftMeta() {
}

void RaftMeta::Init() {
  if (GetCurrentTerm() == 0) {
    SetCurrentTerm(0);
  }
  if (GetVotedForIp() == "") {
    SetVotedForIp("");
  }
  if (GetVotedForPort() == 0) {
    SetVotedForPort(0);
  }
  if (GetCommitIndex() == 0) {
    SetCommitIndex(0);
  }
  if (GetLastApplied() == 0) {
    SetLastApplied(0);
  }
}

uint64_t RaftMeta::GetCurrentTerm() {
  return std::atoll(db_[kCurrentTerm].c_str());
}

void RaftMeta::SetCurrentTerm(const uint64_t current_term) {
  db_[kCurrentTerm] = std::to_string(current_term);
}

std::string RaftMeta::GetVotedForIp() {
  return db_[kVoteForIp];
}

void RaftMeta::SetVotedForIp(const std::string ip) {
  db_[kVoteForIp] = ip;
}

int RaftMeta::GetVotedForPort() {
  return std::atoi(db_[kVoteForPort].c_str());
}

void RaftMeta::SetVotedForPort(const int port) {
  db_[kVoteForPort] = std::to_string(port);
}

uint64_t RaftMeta::GetCommitIndex() {
  return std::atoll(db_[kCommitIndex].c_str());
}

void RaftMeta::SetCommitIndex(uint64_t commit_index) {
  db_[kCommitIndex] = std::to_string(commit_index);
}

uint64_t RaftMeta::GetLastApplied() {
  return std::atoll(db_[kLastApplied].c_str());
}

void RaftMeta::SetLastApplied(uint64_t last_applied) {
  db_[kLastApplied] = std::to_string(last_applied);
}

uint64_t RaftMeta::GetNewFencingToken() {
  uint64_t ans = std::atoll(db_[kFencingToken].c_str());
  ans++;
  db_[kFencingToken] = std::to_string(ans);
  return ans;
}

}  // namespace floyd

// Copyright (c) 2015-present, Qihoo, Inc.  All rights reserved.
// This source code is licensed under the BSD-style license found in the
// LICENSE file in the root directory of this source tree. An additional grant
// of patent rights can be found in the PATENTS file in the same directory.

#include "floyd/src/raft_log.h"

#include <google/protobuf/text_format.h>

#include <vector>
#include <string>

#include "rocksdb/db.h"
#include "rocksdb/iterator.h"
#include "slash/include/xdebug.h"

#include "floyd/src/floyd.pb.h"
#include "floyd/src/logger.h"
#include "floyd/include/floyd_options.h"

namespace floyd {
extern std::string UintToBitStr(const uint64_t num) {
  char buf[8];
  uint64_t num1 = htobe64(num);
  memcpy(buf, &num1, sizeof(uint64_t));
  return std::string(buf, 8);
}

extern uint64_t BitStrToUint(const std::string &str) {
  uint64_t num;
  memcpy(&num, str.c_str(), sizeof(uint64_t));
  return be64toh(num);
}

RaftLog::RaftLog(rocksdb::DB* log, Logger *info_log) :
  db_(),
  info_log_(info_log),
  log_(log),
  is_dump_(false),
  last_log_index_(0) {
  rocksdb::Iterator *it = log_->NewIterator(rocksdb::ReadOptions());
  it->SeekToLast();
  if (it->Valid()) {
    it->Prev();
    it->Prev();
    it->Prev();
    it->Prev();
    it->Prev();
    if (it->Valid()) {
      last_log_index_ = BitStrToUint(it->key().ToString());
      offset_ = last_log_index_;
    }
  }
  delete it;
  dump_log_thread_.set_thread_name("Dump log thread");
  dump_log_thread_.StartThread();
}

RaftLog::~RaftLog() {
}

uint64_t RaftLog::Append(const std::vector<const Entry *> &entries) {
  slash::MutexLock l(&lli_mutex_);
  
  LOGV(DEBUG_LEVEL, info_log_, "RaftLog::Append: entries.size %lld", entries.size());
  // try to commit entries in one batch
  for (size_t i = 0; i < entries.size(); i++) {
    std::string buf;
    entries[i]->SerializeToString(&buf);
    last_log_index_++;
    db_[last_log_index_] = buf;
  }
  assert(last_log_index_ > offset_);
  if (last_log_index_ - offset_ > 10000) {
    dump_log_thread_.Schedule(&DumpLogTaskWrapper, this);
  }
  return last_log_index_;
}

void RaftLog::DumpLogTask() {
  std::map<uint64_t, std::string> dela_db_log;
  while (true) {
    {
      slash::MutexLock l(&lli_mutex_);
      if (last_log_index_ - offset_ < 5000) {
        break;
      }
      auto end = db_.lower_bound(offset_ + 200);
      for (auto iter = db_.begin(); iter != end; ++iter) {
        dela_db_log[iter->first] = iter->second;
      }
      db_.erase(db_.begin(), end);
      (offset_ + 201 == db_.begin()->first);
      offset_ = db_.begin()->first;
    }
    for (auto iter = dela_db_log.begin(); iter != dela_db_log.end(); ++iter) {
      log_->Put(rocksdb::WriteOptions(), std::to_string(iter->first), iter->second);
    }
    dela_db_log.clear();
  }
}

void RaftLog::DumpLogTaskWrapper(void* arg) {
  reinterpret_cast<RaftLog*>(arg)->DumpLogTask();
}

uint64_t RaftLog::GetLastLogIndex() {
  return last_log_index_;
}

int RaftLog::GetEntry(const uint64_t index, Entry *entry) {
  slash::MutexLock l(&lli_mutex_);
  if (index <= offset_) {
    std::string buf = std::to_string(index);
    std::string res;
    rocksdb::Status s = log_->Get(rocksdb::ReadOptions(), buf, &res);
    if (!s.IsNotFound()) {
      entry->ParseFromString(res);
      return 0;
    }
  } 

  if (db_.count(index) == 0) {
    LOGV(ERROR_LEVEL, info_log_, "RaftLog::GetEntry: GetEntry not found, index is %lld", index);
    entry = NULL;
    return 1;
  }
  entry->ParseFromString(db_[index]);
  return 0;
}

bool RaftLog::GetLastLogTermAndIndex(uint64_t* last_log_term, uint64_t* last_log_index) {
  slash::MutexLock l(&lli_mutex_);
  if (last_log_index_ == 0) {
    *last_log_index = 0;
    *last_log_term = 0;
    return true;
  }

  if (db_.count(last_log_index_) != 0) {
    Entry entry;
    entry.ParseFromString(db_[last_log_index_]);
    *last_log_index = last_log_index_;
    *last_log_term = entry.term();
    return true;
  }

  std::string buf = std::to_string(last_log_index_);
  std::string res;
  rocksdb::Status s = log_->Get(rocksdb::ReadOptions(), buf, &res);
  if (!s.IsNotFound()) {
    Entry entry;
    entry.ParseFromString(res);
    *last_log_index = last_log_index_;
    *last_log_term = entry.term();
    return true;
  }

  *last_log_index = 0;
  *last_log_term = 0;

  return true;
}

/*
 * truncate suffix from index
 */
int RaftLog::TruncateSuffix(uint64_t index) {
  // we need to delete the unnecessary entry, since we don't store
  // last_log_index in rocksdb
  slash::MutexLock l(&lli_mutex_);
  rocksdb::WriteBatch batch;
  for (; last_log_index_ >= index; last_log_index_--) {
    if (db_.count(last_log_index_) == 1) {
      db_.erase(last_log_index_);
    } else {
        batch.Delete(UintToBitStr(last_log_index_));
    }
  }
  if (batch.Count() > 0) {
    rocksdb::Status s = log_->Write(rocksdb::WriteOptions(), &batch);
    if (!s.ok()) {
      LOGV(ERROR_LEVEL, info_log_, "RaftLog::TruncateSuffix Error last_log_index %lu "
          "truncate from %lu", last_log_index_, index);
      return -1;
    }
  }
  return 0;
}

}  // namespace floyd

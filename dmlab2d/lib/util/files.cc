// Copyright (C) 2017-2019 The DMLab2D Authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
////////////////////////////////////////////////////////////////////////////////

#include "dmlab2d/lib/util/files.h"

#include <ftw.h>
#include <sys/stat.h>
#include <unistd.h>

#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <iterator>
#include <memory>
#include <string>

namespace deepmind::lab2d::util {

extern "C" {
static int delete_entry(const char* fpath, const struct stat* sb, int typeflag,
                        struct FTW* ftwbuf) {
  return (typeflag == FTW_DP ? rmdir : unlink)(fpath);
}
}  // extern "C"

void RemoveDirectory(const std::string& path) {
  nftw(path.c_str(), delete_entry, 10, FTW_DEPTH | FTW_PHYS);
}

// Recursively build the directories for the folder `path`.
bool MakeDirectory(const std::string& path) {
  struct stat st = {0};
  if (stat(path.c_str(), &st) == 0) {
    return S_ISDIR(st.st_mode);
  }

  auto pos = path.find_last_of('/');
  if (pos == std::string::npos || MakeDirectory(path.substr(0, pos))) {
    mkdir(path.c_str(), 0777);
  }
  return stat(path.c_str(), &st) == 0 && S_ISDIR(st.st_mode);
}

std::string GetTempDirectory() {
  // In unit tests, "TEST_TMPDIR" is the preferred temporary directory.
  const char* tempdir = getenv("TEST_TMPDIR");
  if (tempdir == nullptr) {
    tempdir = getenv("TMPDIR");
    if (tempdir == nullptr) {
      tempdir = "/tmp";
    }
  }
  return tempdir;
}

bool SetContents(const std::string& file_name, absl::string_view contents,
                 const char* scratch_directory) {
  std::string temp_file =
      (scratch_directory != nullptr && scratch_directory[0] != '\0')
          ? std::string(scratch_directory)
          : util::GetTempDirectory();
  temp_file += "/dmlab2d_temp_file_XXXXXX";
  // Thread safe temporary file generation.
  {
    std::unique_ptr<std::FILE, int (&)(std::FILE*)> file(
        fdopen(mkstemp(&temp_file.front()), "w"), std::fclose);
    if (!file) {
      std::cerr << "Failed to make temp file! " << errno << " - "
                << std::strerror(errno) << "\n";
      return false;
    }
    if (std::fwrite(contents.data(), 1, contents.size(), file.get()) !=
        contents.size()) {
      std::cerr << "Failed to write to temp file! " << errno << " - "
                << std::strerror(errno) << "\n";
      return false;
    }
  }

  if (std::rename(temp_file.c_str(), file_name.c_str()) != 0) {
    std::cerr << "Failed to rename temp file to: " << file_name << " " << errno
              << " - " << std::strerror(errno) << "\n";
    std::remove(temp_file.c_str());
    return false;
  }
  return true;
}

bool GetContents(const std::string& file_name, std::string* contents) {
  std::filebuf fb;
  if (fb.open(file_name, std::ios::in | std::ios::binary) == nullptr) {
    return false;
  }
  contents->reserve(fb.pubseekoff(0, std::ios::end));
  fb.pubseekpos(0);
  contents->assign(std::istreambuf_iterator<char>(&fb), {});
  return true;
}

}  // namespace deepmind::lab2d::util

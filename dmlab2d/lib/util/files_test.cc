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

#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#include <cerrno>
#include <cstring>
#include <string>

#include "gtest/gtest.h"

namespace deepmind::lab2d::util {
namespace {

std::string TestName() {
  return ::testing::UnitTest::GetInstance()->current_test_info()->name();
}

// Wrapper expectations for Posix calls that return -1 on error and set errno.
#define EXPECT_POSIX_OK(e) EXPECT_NE(-1, e) << std::strerror(errno)
#define EXPECT_POSIX_ERROR(e) EXPECT_EQ(-1, e)

TEST(FilesTest, RemovesDirectory) {
  // Ensures RemoveDirectory removes directories.
  std::string root_path(GetTempDirectory() + "/" + TestName());
  EXPECT_POSIX_OK(mkdir(root_path.c_str(), S_IRWXU)) << ": " << root_path;
  std::string new_path(root_path + "/b");
  EXPECT_POSIX_OK(mkdir(new_path.c_str(), S_IRWXU)) << ": " << new_path;
  new_path += "/c";
  EXPECT_POSIX_OK(creat(new_path.c_str(), 0)) << ": " << new_path;

  EXPECT_POSIX_OK(access(root_path.c_str(), F_OK)) << ": " << root_path;

  RemoveDirectory(root_path);

  EXPECT_POSIX_ERROR(access(root_path.c_str(), F_OK)) << ": " << root_path;
}

TEST(FilesTest, RemovesDirectoryNoSymlink) {
  // Ensures RemoveDirectory does not follow symlinks.
  std::string temp_directory = GetTempDirectory();
  std::string root_path(GetTempDirectory() + "/" + TestName());
  EXPECT_POSIX_OK(mkdir(root_path.c_str(), S_IRWXU)) << ": " << root_path;
  std::string new_path(root_path + "/b");
  EXPECT_POSIX_OK(mkdir(new_path.c_str(), S_IRWXU)) << ": " << new_path;
  new_path += "/c";
  EXPECT_POSIX_OK(creat(new_path.c_str(), 0)) << ": " << new_path;
  std::string link_path(temp_directory + "/l");
  EXPECT_POSIX_OK(symlink(root_path.c_str(), link_path.c_str()))
      << ": " << root_path << ", " << link_path;
  RemoveDirectory(link_path);
  EXPECT_POSIX_OK(access(root_path.c_str(), F_OK)) << ": " << root_path;
  RemoveDirectory(root_path);
}

TEST(FilesTest, CreateAndRemoveDirectory) {
  // Ensures RemoveDirectory does not follow symlinks.
  std::string root_path(GetTempDirectory() + "/" + TestName());
  std::string new_path(root_path + "/b/b/b");
  EXPECT_TRUE(MakeDirectory(new_path));
  RemoveDirectory(root_path);
}

TEST(FilesTest, CreateFileWithContents) {
  std::string temp_dir = GetTempDirectory();
  std::string root_path(temp_dir + "/dmlab2d_files_test");
  ASSERT_TRUE(MakeDirectory(root_path));

  std::string contents(1000000, '@');
  SetContents(root_path + "/content", contents, temp_dir.c_str());

  std::string result;
  GetContents(root_path + "/content", &result);

  // Don't print large file.
  EXPECT_TRUE(result == contents);

  RemoveDirectory(root_path);
}

}  // namespace
}  // namespace deepmind::lab2d::util

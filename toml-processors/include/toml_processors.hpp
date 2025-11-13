#pragma once

#include "lockfile/lockfile.hpp"
#include "lockfile/lockfile_structure.hpp"

// later add:
// #include "config/config.hpp"
// #include "manifest/manifest.hpp"

namespace toml_processors {
using Lockfile = ::localpm::filesys::LockfileProcessor;
using LockfileError = ::localpm::filesys::LockfileError;
using LockfileErrorCode = ::localpm::filesys::LockfileErrorCode;

using ::localpm::filesys::Compiler;
using ::localpm::filesys::Package;
using ::localpm::filesys::Project;
using ::localpm::filesys::SrcType;
} // namespace toml_processors

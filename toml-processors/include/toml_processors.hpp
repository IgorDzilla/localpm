#pragma once

#include "lockfile/lockfile.hpp"
#include "lockfile/lockfile_structure.hpp"

#include "manifest/manifest.hpp"
#include "manifest/manifest_structure.hpp"

// later add:
// #include "config/config.hpp"

namespace toml_processors {
// Lockfile API
using Lockfile = ::localpm::filesys::LockfileProcessor;
using LockfileError = ::localpm::filesys::LockfileError;
using LockfileErrorCode = ::localpm::filesys::LockfileErrorCode;

using ::localpm::filesys::Compiler;
using ::localpm::filesys::Package;
using ::localpm::filesys::Project;
using ::localpm::filesys::SrcType;

// Manifest API
using Manifest = ::localpm::manifest::Manifest;
using ManifestProcessor = ::localpm::filesys::ManifestProcessor;
} // namespace toml_processors

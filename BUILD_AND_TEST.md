# Build and Test Guide

## Quick Build

```bash
# From project root
mkdir build && cd build
cmake ..
cmake --build .
```

Binary will be at: `build/bin/localpm`

## Test Commands

### 1. Check help:
```bash
./bin/localpm --help
```

Should show all registered commands (init, add, list, install).

### 2. Test init command:
```bash
./bin/localpm init
./bin/localpm init --dir /tmp/test --force
```

### 3. Test add command:
```bash
./bin/localpm add fmt --version 10.2.1
./bin/localpm add mylib --version latest --source local --path ./vendor/mylib
```

### 4. Test list command:
```bash
./bin/localpm list
```

### 5. Test install command:
```bash
./bin/localpm install fmt
```

### 6. Test verbose mode:
```bash
./bin/localpm -v init
./bin/localpm --verbose add mylib
```

## Test with Lockfile

Create a test lockfile `test.lock`:

```toml
schema = 1

[project]
name = "test-project"
version = "1.0.0"

[project.compiler]
cc = "gcc"
cflags = ["-Wall", "-O2"]

[[packages]]
name = "fmt"
version = "10.2.1"
type = "local"
kind = "header-only"
```

Then test lockfile parsing (you'll need a command that reads lockfiles):

```bash
./bin/localpm status --file test.lock  # if you add the status command
```

## Module Testing

### Test CLI module standalone:
```bash
cd cli
mkdir build && cd build
cmake -DCLI_BUILD_STANDALONE=ON ..
cmake --build .
./localpm-cli --help
```

### Test lockfile module standalone:
```bash
cd lockfile
mkdir build && cd build
cmake -DLOCKFILE_BUILD_STANDALONE=ON ..
cmake --build .
./lockfile_test
```

## Build Options

### Release build:
```bash
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build .
```

### Disable tests:
```bash
cmake -DLOCALPM_BUILD_TESTS=OFF ..
```

### Disable apps:
```bash
cmake -DLOCALPM_BUILD_APPS=OFF ..
```

### Enable standalone module builds:
```bash
cmake -DCLI_BUILD_STANDALONE=ON -DLOCKFILE_BUILD_STANDALONE=ON ..
```

## Troubleshooting

### Missing modules?
If cli or lockfile directories are missing, bring them from scratches branch:
```bash
git checkout scratches -- cli/ lockfile/
```

### Clean rebuild:
```bash
rm -rf build
mkdir build && cd build
cmake ..
cmake --build .
```

### Check what was built:
```bash
ls -R build/bin/
ls -R build/lib/
```

### Verbose build to see compilation:
```bash
cmake --build . --verbose
```

## Expected Output

After successful build:
- `build/bin/localpm` - main executable
- `build/lib/liblockfile.a` - lockfile static library (if built)
- Optional: `build/bin/localpm-cli` - CLI standalone (if enabled)
- Optional: `build/bin/lockfile_test` - lockfile test (if enabled)

## Integration Test Example

Create a script `test_all.sh`:

```bash
#!/bin/bash
set -e

echo "Building project..."
mkdir -p build && cd build
cmake ..
cmake --build .

echo -e "\n=== Testing help ==="
./bin/localpm --help

echo -e "\n=== Testing init ==="
./bin/localpm init --dir /tmp/localpm_test

echo -e "\n=== Testing add ==="
./bin/localpm add fmt --version 10.2.1

echo -e "\n=== Testing list ==="
./bin/localpm list

echo -e "\nAll tests passed!"
```

Run it:
```bash
chmod +x test_all.sh
./test_all.sh
```

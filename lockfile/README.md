#Lockfile

`Lockfile` is core part of any project using `localpm`. It holds all information about packages and dependencies required to build the project. File format is `.toml` for convenience and simplicity of editing and reading.

## Lockfile structure

`sample.toml`
```
[lockfile]
schema = 1

[project]
name = "demo"
version = "0.1.0"

[project.compiler]
cc="gcc-14"
cflags=["-O3","-Wall"]
ldflags=["-s"]

[[packages]]
name="fmt"
version="10.2.1"
kind = "static"
type = "local"
source = {path="some/path/to/lib"}
integrity={ tarball_sha256="9f..cd" }

[[packages]]
name="demo"
version="0.1.0"
kind = "shared"
type = "local"
dependencies=[ { name="fmt", version="^10.2", resolved="10.2.1" } ]

[[packages]]
name="some-lib"
version="latest"
type = "local"
kind = "header-only"

```

## `[lockfile]`
Duty table. Will be automatically generated under any circumstances.

### `[project]`
Basic project information.

| Field   | Description            | Type   | Status                                  |
| ------- | ---------------------- | ------ | --------------------------------------- |
| name    | Project name           | string | Yes                                     |
| version | Version of the project | string | If not provided, will be set to "0.1.0" |

### `[project].compiler`
Information about compiler, build system and user required compile and linker flags.

| Field        | Description          | Type             | Status    |
| ------------ | -------------------- | ---------------- | --------- |
| cc           | Compiler name        | string           | Mandatory |
| cflags       | Compile flags        | array of strings | Optional  |
| ldflags      | Linker flags         | array of string  | Optional  |
| build_system | Name of build system | string           | Optional  |

> [!INFO] Build systems
> Currently no build systems are supported, only basic compilation with `cc`.
> Goal is to make `localpm` compatible with `cmake`.


### `[[packages]]`
Array of tables. Each holds info about project.

| Field        | Description                 | Type                       | Status                                     |
| ------------ | --------------------------- | -------------------------- | ------------------------------------------ |
| name         | Package name                | string                     | Mandatory                                  |
| version      | Version of the package      | string                     | Mandatory                                  |
| kind         | Kind of package             | string: one of given kinds | Optional *                                 |
| type         | Package's source type       | string: one of given types | Optional *                                 |
| dependencies | Required dependencies       | array of inline tables     | Optional                                   |
| source       | Info about package's source | inline table               | Mandatory unless type is local or registry |
| Integrity    | Package's integrity         | inline table               | Optional                                   |
#### `[packages.*].dependencies`
| Field    | Description | Type   | Status    |
| -------- | ----------- | ------ | --------- |
| name     | name        | string | Mandatory |
| version  | version     | string | Mandatory |
| resolved | -           | string | Optional  |
#### `[packages.*].source`
Currently empty.

#### `[packages.*].integrity`
| Field          | Description | Type   | Status    |
| -------------- | ----------- | ------ | --------- |
| tarball_sha256 | Cheksum     | string | Mandatory |


## Ways of generating lockfile
### Automatic
Run CLI commands generate `locfile.toml`. 

Running `localpm init` will generate basic template.
```
localpm init [args]
	[args]
		--template="<template name>" # from template
		--schema=<schema number>     # from basic shemas
		
```

Add packages:
```
localpm add <package_name>@<version> [args] # adds package with given name
	[args]
		--source=git/archive/fodler/registry
```
More info about CLI commands you can get in corresponding page of docs.

### Manual
You can make these files yourself. In an order to verify `lockfile.toml` you can use this command:
```
localpm checkheath --lockfile
```

If any errors exist, output will tell you.

# DS FileSystem Project

## Usage

### Prerequisites

You will need:

 * A modern C/C++ compiler
 * CMake 3.2+ installed (on a Mac, run `brew install cmake`)

### Building The Project

#### Git Clone

First we need to check out the git repo:

```bash
❯ mkdir ~/workspace
❯ cd ~/workspace
❯ git clone \
    https://github.com/DATA-STRUCT-PROJET/PROJECT \
    ds-project
```

#### Project Structure

There are three empty folders: `lib`, `bin`, and `include`. Those are populated by `make install`.

The rest should be obvious: `src` is the sources, and `test` is where we put our unit tests.

Now we can build this project, and below we show three separate ways to do so.

#### Building Manually

```bash
❯ rm -rf build && mkdir build
❯ git submodule init && git submodule update
❯ cd build
❯ cmake ..
❯ make
```


#### Running the tests

```bash
❯ build/bin/fs
<TODO run example>
```

#### Running the CLI Executable

```bash
❯ bin/prompt
```
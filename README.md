WIP.

[![TravisCI Build Status](https://travis-ci.org/ligfx/cmake-format.svg?branch=master)](https://travis-ci.org/ligfx/cmake-format)
[![AppVeyor Build Status](https://ci.appveyor.com/api/projects/status/yjeg0k27l9lbyd3v?svg=true)](https://ci.appveyor.com/project/ligfx/cmake-format)

```
$ ./cmake-format -help
usage: ./cmake-format [options] [file ...]

Re-formats specified files. If no files are specified on the command-line,
reads from standard input. If -i is specified, formats files in-place;
otherwise, writes results to standard output.

options:
  -column-limit=NUMBER               Set maximum column width to NUMBER. If ReflowArguments is None, this does nothing.
  -command-case=CASE                 Letter case of command invocations. Available: lower, upper
  -continuation-indent-width=NUMBER  Indent width for line continuations.
  -indent-width=NUMBER               Use NUMBER spaces for indentation.
  -max-empty-lines-to-keep=NUMBER    The maximum number of consecutive empty lines to keep.
  -reflow-arguments=ALGORITHM        Algorithm to reflow command arguments. Available: none, oneperline, binpack, heuristic
  -space-before-parens=CONDITION     When to put a space before opening parentheses. Available: always, controlstatements, never
  -i                                 Re-format files in-place.
  -q                                 Quiet mode: suppress informational messages.
  -self-test                         Run built-in test suite. This must be the first argument; all others are passed to the test runner.
```

TODO:
- [ ] Enforce maximum column width (moving arguments between lines + splitting up arguments to message)
  - [x] Put one argument per line
  - [x] Reflow arguments naively
  - [ ] Reflow breaking before boolean operators like AND
  - [ ] Figure out what a function "looks like"; does it have a run of filenames? does it start with an unquoted argument, then a keyword, then a bunch of not-uppercase stuff? is an argument a command-line option? etc.
- [ ] Do not specify content in CMake 'else()' parens
- [x] lower-case command names in macro/function
- [ ] check CMAKE_SOURCE_DIR/CMAKE_BINARY_DIR vs PROJECT_SOURCE_DIR/PROJECT_BINARY_DIR
- [ ] sort items in SET() and LIST(APPEND) and repeated functions (like add_subdirectory, add_definitions, etc)
- [ ] variables in IF() don't always need to be a variable reference (there's some CMake policy about this, how's it actually work?)
- [ ] convert an unquoted argument with double-quotes, into a quoted argument
- [ ] use LIST(APPEND) and STRING(APPEND) instead of SET(VAR ${VAR} ...)
- [x] no spaces between command identifiers and opening-parenthesis
- [ ] no blank lines at end of block
- [x] maximum blank lines in a row
- [ ] remove unneeded double quotes around variable references
- [ ] support bracket comments

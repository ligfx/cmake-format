WIP.

```
$ ./cmake-format -help
usage: ./cmake-format [options] [file ...]

Re-formats specified files. If -i is specified, formats files
in-place; otherwise, writes results to standard output.

options:
  -lowercase-commands            Lowercase command names in command invocations.
  -indent=STRING                 Use STRING for indentation.
  -indent-arguments=align-paren  Align arguments on continuing lines with the command's left parenthesis.
  -indent-arguments=STRING       Use STRING for indenting arguments on continuing lines.
  -indent-rparen=STRING          Use STRING for indenting hanging right-parens.
  -i                             Re-format files in-place.
```

TODO:
- [ ] Enforce maximum column width (moving arguments between lines + splitting up arguments to message)
- [ ] Do not specify content in CMake 'else()' parens
- [ ] lower-case command names in macro/function
- [ ] check CMAKE_SOURCE_DIR/CMAKE_BINARY_DIR vs PROJECT_SOURCE_DIR/PROJECT_BINARY_DIR
- [ ] sort items in SET() and LIST(APPEND) and repeated functions (like add_subdirectory, add_definitions, etc)
- [ ] variables in IF() don't always need to be a variable reference (there's some CMake policy about this, how's it actually work?)
- [ ] convert an unquoted argument with double-quotes, into a quoted argument
- [ ] use LIST(APPEND) and STRING(APPEND) instead of SET(VAR ${VAR} ...)
- [ ] no spaces between command identifiers and opening-parenthesis
- [ ] no blank lines at end of block
- [ ] maximum blank lines in a row
- [ ] remove unneeded double quotes around variable references
- [ ] support bracket comments

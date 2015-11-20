# Library avian #

`#include <avian.h>`

<dl>
<dd><a href='Overview.md'>Overview</a></dd>
<dd><a href='Index.md'>Index</a></dd>
<dd><a href='Examples.md'>Examples</a></dd>
</dl>

## Overview ##

## Index ##

<dl>
<dd><a href='Constants.md'>Constants</a></dd>
<dd><a href='Variables.md'>Variables</a></dd>
<dd><a href='Macros.md'>Macros</a></dd>
<dd><a href='Typedefs.md'>Typedefs</a></dd>
<dd><a href='Functions.md'>Functions</a></dd>
</dl>

### Constants ###
### Variables ###
```
char *arg0 // program name
int rval   // return value
```
### Macros ###
### Typedefs ###
### Functions ###
```
void alert(char *fmt, ...)
```

Print an error message to `stderr` and set `rval` to `EXIT_FAILURE`,
the format of the message is as if passed to `printf`.

```
void fatal(int ec, char *fmt, ...)
```

Same as `alert`, but exits with exit code `ec`.

[src/libavian/fatal.c](https://code.google.com/p/avianos/source/browse/src/libavian/fatal.c)

## Examples ##
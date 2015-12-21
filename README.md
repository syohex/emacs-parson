# parson

Binding of parson JSON parser.

## Requirement

This package uses Emacs 25 dynamic module feature.


## API

### `(parson-parse string)`

Parse JSON string `string` and return Emacs Lisp object.

### `(parson-stringify object)`

Encode `object` into JSON.

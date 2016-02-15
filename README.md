# parson

Binding of parson JSON parser.

## Requirement

This package uses Emacs 25 dynamic module feature.


## API

### `(parson-parse string)`

Parse JSON string `string` and return Emacs Lisp object.

### `(parson-stringify object)`

Encode `object` into JSON.


## Benchmark(parson vs json.el standard library)

- **Y-axis means second.**
- **Lower is better**

Benchmark code is under `bench/`.

#### Encoding scalar 100,000 times

![encoding-scalar](image/encode-scalar.png)

#### Encoding 5 elements list 100,000 times

![encoding-list](image/encode-list.png)

#### Encoding [complex object](bench/bench.json) 10,000 times

![encoding-object](image/encode-object.png)

#### Decoding empty list 100,000 times

![encoding-list](image/decode-list.png)

#### Decoding empty object 100,000 times

![encoding-object](image/decode-object.png)


## License

- This software is released under the GPLv3.
- parson is released under the MIT License

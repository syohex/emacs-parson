# parson

Binding of parson JSON parser.

## Requirement

This package uses Emacs 25 dynamic module feature.


## API

### `(parson-parse string)`

Parse JSON string `string` and return Emacs Lisp object.

### `(parson-stringify object)`

Encode `object` into JSON.


## Benchmark(parson vs json standard library)

### Encode into JSON

Encoding with parson is very slower than json.el.

```lisp
;; Elapsed time: 2.457449s (1.600100s in 240 GCs)
(benchmark 1000000 '(json-encode [t]))

;; Elapsed time: 7.456625s (1.547315s in 230 GCs)
(benchmark 1000000 '(parson-stringify [t]))
```

```lisp
(setq hash (make-hash-table))

;; Elapsed time: 1.926598s (0.805901s in 120 GCs)
(benchmark 1000000 '(json-encode hash))

;; Elapsed time: 12.183582s (2.782884s in 350 GCs)
(benchmark 1000000 '(parson-stringify hash))
```

### Decode from JSON

```lisp
;; Elapsed time: 17.331068s (3.556868s in 460 GCs)
(benchmark 1000000 '(json-read-from-string "[true]"))

;; Elapsed time: 8.857157s (7.057974s in 270 GCs)
(benchmark 1000000 '(parson-parse "[true]"))
```

```lisp
;; Elapsed time: 34.019030s (23.648462s in 480 GCs)
(benchmark 1000000 '(json-read-from-string "{}"))


;; Elapsed time: 13.595240s (11.947698s in 251 GCs)
(benchmark 1000000 '(parson-parse "{}"))
```

## License

- This software is released under the GPLv3.
- parson is released under the MIT License

;;; test.el --- parson binding test

;; Copyright (C) 2015 by Syohei YOSHIDA

;; Author: Syohei YOSHIDA <syohex@gmail.com>

;; This program is free software; you can redistribute it and/or modify
;; it under the terms of the GNU General Public License as published by
;; the Free Software Foundation, either version 3 of the License, or
;; (at your option) any later version.

;; This program is distributed in the hope that it will be useful,
;; but WITHOUT ANY WARRANTY; without even the implied warranty of
;; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;; GNU General Public License for more details.

;; You should have received a copy of the GNU General Public License
;; along with this program.  If not, see <http://www.gnu.org/licenses/>.

;;; Commentary:

;;; Code:

(require 'ert)
(require 'parson)

(ert-deftest parse-object ()
  "Parse object"
  (should (equal (parson-parse "{\"foo\": \"bar\"}") '(("foo" . "bar")))))

(ert-deftest parse-null ()
  "Parse 'null'"
  (should (equal (parson-parse "{\"foo\": null}") '(("foo" . nil)))))

(ert-deftest parse-array ()
  "Parse array"
  (should (string= (aref (parson-parse "[true, \"foo\"]") 1) "foo")))

(ert-deftest parse-multi-byte ()
  "Parse multibyte"
  (should (equal (parson-parse "{\"あいうえお\": \"かきくけこ\"}")
                 '(("あいうえお" . "かきくけこ")))))

(ert-deftest stringify-boolean ()
  "Stringify boolean"
  (should (string= (parson-stringify 't) "true")))

(ert-deftest stringify-symbol ()
  "Stringify symbol"
  (should (string= (parson-stringify 'symbol) "symbol")))

(ert-deftest stringify-object-with-string-value ()
  "Stringify object which has string value"
  (let ((hash (make-hash-table)))
    (puthash "foo" "bar" hash)
    (should (string= (parson-stringify hash) "{\"foo\":\"bar\"}"))))

(ert-deftest stringify-object-with-numeric-value ()
  "Stringify object which has numeric value"
  (let ((hash (make-hash-table)))
    (puthash "foo" 1 hash)
    (should (string= (parson-stringify hash) "{\"foo\":1}"))))

(ert-deftest stringify-object-with-float-value ()
  "Stringify object which has float value"
  (let ((hash (make-hash-table)))
    (puthash "foo" 0.5 hash)
    (should (string= (parson-stringify hash) "{\"foo\":0.5}"))))

(ert-deftest stringify-object-with-nil-value ()
  "Stringify object which has 'nil'"
  (let ((hash (make-hash-table)))
    (puthash "foo" nil hash)
    (should (string= (parson-stringify hash) "{\"foo\":null}"))))

(ert-deftest stringify-object-with-boolean-key-and-float-value ()
  "Stringify object which has boolean key and float value"
  (let ((hash (make-hash-table)))
    (puthash 'true 5.0 hash)
    (should (string= (parson-stringify hash) "{\"true\":5}"))))

(ert-deftest stringify-object-with-object-key-and-float-value ()
  "Stringify object which has object key and float value"
  (let ((hash (make-hash-table))
        (keyhash (make-hash-table)))
    (puthash "foo" "bar" keyhash)
    (puthash keyhash 1.5 hash)
    (should (string= (parson-stringify hash) "{{\"foo\":\"bar\"}:1.5}"))))

(ert-deftest stringify-empty-array ()
  "Stringify empty array"
  (should (string= (parson-stringify []) "[]")))

(ert-deftest stringify-array-with-few-elements ()
  "Stringify array which has few elements"
  (should (string= (parson-stringify [1 t "foo"]) "[1,true,\"foo\"]")))

(ert-deftest stringify-object-with-several-keys ()
  "Stringify object which has several keys"
  (let ((hash (make-hash-table)))
    (puthash "foo" 1 hash)
    (puthash "bar" 2 hash)
    (let ((got (parson-stringify hash)))
      (should (or (string= "{\"foo\":1,\"bar\":2}" got)
                  (string= "{\"bar\":2,\"foo\":1}" got))))))

(ert-deftest stringify-multi-byte()
  "Stringify multiple byte characters"
  (should (string= (parson-stringify ["あいうえお"]) "[\"あいうえお\"]")))

(ert-deftest stringify-escaped()
  "Stringify escaped characters"
  (should (string= (parson-stringify ["\\"]) "[\"\\\\\"]")))

(ert-deftest stringify-escaped-quote ()
  "Stringify escaped quote"
  (should (string= (parson-stringify ["\\"]) "[\"\\\\\"]"))
  (should (string= (parson-stringify ["\n"]) "[\"\\n\"]"))
  (should (string= (parson-stringify ["\r"]) "[\"\\r\"]"))
  (should (string= (parson-stringify ["\f"]) "[\"\\f\"]"))
  (should (string= (parson-stringify ["\t"]) "[\"\\t\"]"))
  (should (string= (parson-stringify ["\b"]) "[\"\\b\"]")))

;;; test.el ends here

;;; bench.el --- benchmark of parson

;; Copyright (C) 2016 by Syohei YOSHIDA

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

(require 'parson)
(require 'json)

(defvar test-json
  (with-current-buffer (find-file-noselect
                        (concat
                         (if load-file-name
                             (file-name-directory load-file-name)
                           default-directory)
                         "bench.json"))
    (buffer-string)))

(message "Encode Scalar: Person")
(benchmark 100000 '(parson-stringify 0))
(message "Encode Scalar: json.el")
(benchmark 100000 '(json-encode 0))

(message "Encode vector: Person")
(benchmark '100000 '(parson-stringify [1 2 3 4 5]))
(message "Encode vector: json.el")
(benchmark '100000 '(json-encode [1 2 3 4 5]))

(message "Encode complex object: Person")
(benchmark 10000 '(parson-parse test-json))
(message "Encode complex object: json.el")
(benchmark 10000 '(json-read-from-string test-json))

(message "Decore vector: Person")
(benchmark 100000 '(parson-parse "[true]"))
(message "Decore vector: json.el")
(benchmark 100000 '(json-read-from-string "[true]"))

(message "Decore object: Person")
(benchmark 100000 '(parson-parse "{}"))
(message "Decore object: json.el")
(benchmark 100000 '(json-read-from-string "{}"))


;;; bench.el ends here

/*
  Copyright (C) 2015 by Syohei YOSHIDA

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <emacs-module.h>

#include "parson.h"

int plugin_is_GPL_compatible;

//#define DEBUG
#ifdef DEBUG
static void
dump(emacs_env *env, emacs_value v)
{
	emacs_value Qmessage = env->intern(env, "message");
	emacs_value q_args[2] = { env->make_string(env, "%s", 2), v };
	env->funcall(env, Qmessage, 2, q_args);
}
#endif

static bool
eq_type(emacs_env *env, emacs_value type, const char *type_str)
{
	return env->eq(env, type, env->intern(env, type_str));
}

static intmax_t
list_length(emacs_env *env, emacs_value seq)
{
	emacs_value Qlength = env->intern(env, "length");
	emacs_value args[1] = { seq };

	emacs_value len = env->funcall(env, Qlength, 1, args);
	return env->extract_integer(env, len);
}

static emacs_value
list_nth(emacs_env *env, emacs_value seq, intmax_t index)
{
	emacs_value Qnth = env->intern(env, "nth");
	emacs_value i = env->make_integer(env, index);
	emacs_value args[2] = { i, seq };

	return env->funcall(env, Qnth, 2, args);
}

static emacs_value
hash_keys(emacs_env *env, emacs_value hash)
{
	emacs_value Qhash_table_keys = env->intern(env, "hash-table-keys");
	emacs_value args[1] = { hash };

	return env->funcall(env, Qhash_table_keys, 1, args);
}

static emacs_value
hash_value(emacs_env *env, emacs_value hash, emacs_value key)
{
	emacs_value Qgethash = env->intern(env, "gethash");
	emacs_value args[2] = { key, hash };

	return env->funcall(env, Qgethash, 2, args);
}

static intmax_t
emacs_value_to_string(emacs_env *env, emacs_value value, char *ptr)
{
	emacs_value type = env->type_of(env, value);

	if (!env->is_not_nil(env, value)) {
		memcpy(ptr, "null", 4);
		return 4;
	} else if (env->eq(env, value, env->intern(env, "t"))) {
		memcpy(ptr, "true", 4);
		return 4;
	} else if (eq_type(env, type, "integer")) {
		char b[32];
		int step = snprintf(b, sizeof(b), "%ld", env->extract_integer(env, value));
		memcpy(ptr, b, step);
		return step;
	} else if (eq_type(env, type, "float")) {
		char b[64];
		int step = snprintf(b, sizeof(b), "%g", env->extract_float(env, value));
		memcpy(ptr, b, step);
		return step;
	} else if (eq_type(env, type, "symbol")) {
		emacs_value Qsymbol_name = env->intern(env, "symbol-name");
		emacs_value args[1] = { value };
		emacs_value name = env->funcall(env, Qsymbol_name, 1, args);

		ptrdiff_t len = 0;
		env->copy_string_contents(env, name, NULL, &len);
		env->copy_string_contents(env, name, ptr, &len);

		return (intmax_t)(len-1);
	} else if (eq_type(env, type, "string")) {
		emacs_value str = value;
		ptrdiff_t len = 0;

		env->copy_string_contents(env, str, NULL, &len);
		char *str_buf = malloc(len);
		env->copy_string_contents(env, str, str_buf, &len);

		char *p = str_buf;
		char *buf = malloc(len * 2);
		size_t count = 0;

		buf[count++] = '\"';

		while (*p != '\0') {
			switch (*p) {
			case '\\':
				buf[count++] = '\\';
				buf[count++] = '\\';
				break;
			case '"':
				buf[count++] = '\\';
				buf[count++] = '\"';
				break;
			case '\b':
				buf[count++] = '\\';
				buf[count++] = 'b';
				break;
			case '\f':
				buf[count++] = '\\';
				buf[count++] = 'f';
				break;
			case '\n':
				buf[count++] = '\\';
				buf[count++] = 'n';
				break;
			case '\r':
				buf[count++] = '\\';
				buf[count++] = 'r';
				break;
			case '\t':
				buf[count++] = '\\';
				buf[count++] = 't';
				break;
			default:
				buf[count++] = *p;
				break;
			}

			++p;
		}

		buf[count++] = '\"';
		buf[count] = '\0';

		memcpy(ptr, buf, count);
		free(buf);
		free(str_buf);
		return count;
	} else if (eq_type(env, type, "vector")) {
		emacs_value vec = value;
		intmax_t len = env->vec_size(env, vec);

		char *orig = ptr;
		*ptr++ = '[';

		for (ptrdiff_t i = 0; i < len; ++i) {
			ptr += emacs_value_to_string(env, env->vec_get(env, vec, i), ptr);
			if (i + 1 != len)
				*ptr++ = ',';
		}

		*ptr++ = ']';
		return (intmax_t)(ptr - orig);
	} else if (eq_type(env, type, "hash-table")) {
		emacs_value hash = value;
		emacs_value keys = hash_keys(env, hash);
		intmax_t size = list_length(env, keys);

		char *orig = ptr;
		*ptr++ = '{';

		for (intmax_t i = 0; i < size; ++i) {
			emacs_value k = list_nth(env, keys, i);
			emacs_value v = hash_value(env, hash, k);

			intmax_t len = emacs_value_to_string(env, k, ptr);
			ptr += len;

			*ptr++ = ':';

			len = emacs_value_to_string(env, v, ptr);
			ptr += len;

			if (i + 1 != size)
				*ptr++ = ',';
		}

		*ptr++ = '}';
		return (intmax_t)(ptr - orig);
	}

	return 0;
}

static emacs_value
json_value_to_emacs_value(emacs_env *env, JSON_Value* value)
{
	switch (json_value_get_type(value)) {
	case JSONError:
	case JSONNull:
		return env->intern(env, "nil");
	case JSONString: {
		const char *str = json_value_get_string(value);
		return env->make_string(env, str, strlen(str));
	}
	case JSONNumber: {
		double d = json_value_get_number(value);
		if (floor(d) == d)
			return env->make_integer(env, (intmax_t)d);
		else
			return env->make_float(env, d);
	}
	case JSONObject: {
		JSON_Object *obj = json_value_get_object(value);
		size_t len = json_object_get_count(obj);

		emacs_value *pairs = malloc(sizeof(emacs_value) * len);
		if (pairs == NULL)
			return env->intern(env, "nil");

		for (size_t i = 0; i < len; ++i) {
			const char *name = json_object_get_name(obj, i);
			JSON_Value *v = json_object_get_value(obj, name);

			emacs_value Qcons = env->intern(env, "cons");
			emacs_value cons_args[2];
			cons_args[0] = env->make_string(env, name, strlen(name));
			cons_args[1] = json_value_to_emacs_value(env, v);

			pairs[i] = env->funcall(env, Qcons, 2, cons_args);
		}

		emacs_value ret = env->funcall(env, env->intern(env, "list"), len, pairs);
		free(pairs);
		return ret;
	}
	case JSONArray: {
		JSON_Array *array = json_value_get_array(value);
		size_t len = json_array_get_count(array);

		emacs_value *elements = malloc(sizeof(emacs_value) * len);
		if (elements == NULL)
			return env->intern(env, "nil");

		for (size_t i = 0; i < len; ++i) {
			JSON_Value *elem = json_array_get_value(array, i);
			elements[i] = json_value_to_emacs_value(env, elem);
		}

		emacs_value Fvector = env->intern(env, "vector");
		emacs_value vec = env->funcall(env, Fvector, len, elements);

		free(elements);
		return vec;
	}
	case JSONBoolean:
		if (json_value_get_boolean(value))
			return env->intern(env, "t");
		else
			return env->intern(env, "nil");
	default:
		env->non_local_exit_signal(env, env->intern(env, "error"),
					   env->make_integer(env, -1));
	}

	return env->intern(env, "nil");
}

static emacs_value
Fperson_stringify(emacs_env *env, ptrdiff_t nargs, emacs_value args[], void *data)
{
	// TODO: realloc
	char *buffer = (char*)malloc(1 << 20);
	intmax_t len = emacs_value_to_string(env, args[0], buffer);

	emacs_value ret = env->make_string(env, buffer, len);
	free(buffer);
	return ret;
}

static emacs_value
Fperson_parse(emacs_env *env, ptrdiff_t nargs, emacs_value args[], void *data)
{
	emacs_value str = args[0];
	ptrdiff_t size = 0;
	char *buf = NULL;

	env->copy_string_contents(env, str, buf, &size);
	buf = malloc(size);
	env->copy_string_contents(env, str, buf, &size);

	JSON_Value *root_value = json_parse_string(buf);
	if (root_value == NULL) {
		return env->intern(env, "nil");
	}

	emacs_value value = json_value_to_emacs_value(env, root_value);
	json_value_free(root_value);

	return value;
}

static void
bind_function(emacs_env *env, const char *name, emacs_value Sfun)
{
	emacs_value Qfset = env->intern(env, "fset");
	emacs_value Qsym = env->intern(env, name);
	emacs_value args[] = { Qsym, Sfun };

	env->funcall(env, Qfset, 2, args);
}

static void
provide(emacs_env *env, const char *feature)
{
	emacs_value Qfeat = env->intern(env, feature);
	emacs_value Qprovide = env->intern (env, "provide");
	emacs_value args[] = { Qfeat };

	env->funcall(env, Qprovide, 1, args);
}

int
emacs_module_init(struct emacs_runtime *ert)
{
	emacs_env *env = ert->get_environment(ert);

#define DEFUN(lsym, csym, amin, amax, doc, data) \
	bind_function(env, lsym, env->make_function(env, amin, amax, csym, doc, data))

	DEFUN("parson-parse", Fperson_parse, 1, 1, "Parse JSON", NULL);
	DEFUN("parson-stringify", Fperson_stringify, 1, 1, "Stringify JSON to Emacs Lisp object", NULL);

#undef DEFUN

	provide(env, "parson-core");
	return 0;
}

/*
  Local Variables:
  c-basic-offset: 8
  indent-tabs-mode: t
  End:
*/

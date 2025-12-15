/*
    __ _____ _____ _____
 __|  |   __|     |   | |  JSON for Modern C++
|  |  |__   |  |  | | | |  version 3.11.3
|_____|_____|_____|_|___|  https://github.com/nlohmann/json

Licensed under the MIT License <http://opensource.org/licenses/MIT>.
SPDX-License-Identifier: MIT
Copyright (c) 2013-2022 Niels Lohmann <http://nlohmann.me>.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#pragma once

#include <nlohmann/json.hpp>
#include <pybind11/pybind11.h>

namespace py = pybind11;

// #define PYJSON_HAS_STRING_VIEW
// #if defined(__cpp_lib_string_view) && __cpp_lib_string_view >= 201606L
// #    define PYJSON_HAS_STRING_VIEW
// #endif

// #ifdef PYJSON_HAS_STRING_VIEW
// #    include <string_view>
// #endif

namespace pyjson
{

/**
 * @brief Create a `nlohmann::json` object from a Python object.
 *
 * @param obj The Python object to convert.
 * @return The `nlohmann::json` object.
 */
nlohmann::json from_python(const py::object &obj);

/**
 * @brief Create a Python object from a `nlohmann::json` object.
 *
 * @param j The `nlohmann::json` object to convert.
 * @return The Python object.
 */
py::object to_python(const nlohmann::json &j);

} // namespace pyjson

namespace nlohmann
{

template <> struct adl_serializer<py::object> {
    static void to_json(json &j, const py::object &obj) { j = pyjson::from_python(obj); }

    static void from_json(const json &j, py::object &obj) { obj = pyjson::to_python(j); }
};

} // namespace nlohmann

namespace pyjson
{

inline nlohmann::json from_python(const py::object &obj)
{
    if (obj.is_none()) {
        return nullptr;
    }
    if (py::isinstance<py::bool_>(obj)) {
        return obj.cast<bool>();
    }
    if (py::isinstance<py::int_>(obj)) {
        return obj.cast<long long>();
    }
    if (py::isinstance<py::float_>(obj)) {
        return obj.cast<double>();
    }
// #ifdef PYJSON_HAS_STRING_VIEW
//     if (py::isinstance<py::str>(obj)) {
//         return std::string_view(PyUnicode_AsUTF8(obj.ptr()), PyUnicode_GET_LENGTH(obj.ptr()));
//     }
// #else
    if (py::isinstance<py::str>(obj)) {
        return obj.cast<std::string>();
    }
// #endif
    if (py::isinstance<py::tuple>(obj) || py::isinstance<py::list>(obj)) {
        auto j = nlohmann::json::array();
        for (const auto &item : obj.cast<py::list>()) {
            j.push_back(from_python(py::reinterpret_borrow<py::object>(item)));
        }
        return j;
    }
    if (py::isinstance<py::dict>(obj)) {
        auto j = nlohmann::json::object();
        for (const auto &item : obj.cast<py::dict>()) {
            j[py::str(item.first)] = from_python(py::reinterpret_borrow<py::object>(item.second));
        }
        return j;
    }

    throw std::runtime_error("Unsupported type for conversion to json");
}

inline py::object to_python(const nlohmann::json &j)
{
    if (j.is_null()) {
        return py::none();
    }
    if (j.is_boolean()) {
        return py::bool_(j.get<bool>());
    }
    if (j.is_number_integer()) {
        return py::int_(j.get<long long>());
    }
    if (j.is_number_float()) {
        return py::float_(j.get<double>());
    }
    if (j.is_string()) {
        return py::str(j.get<std::string>());
    }
    if (j.is_array()) {
        py::list obj;
        for (const auto &item : j) {
            obj.append(to_python(item));
        }
        return obj;
    }
    if (j.is_object()) {
        py::dict obj;
        for (const auto &item : j.items()) {
            obj[py::str(item.key())] = to_python(item.value());
        }
        return obj;
    }

    throw std::runtime_error("Unsupported type for conversion to python object");
}

} // namespace pyjson

PYBIND11_NAMESPACE_BEGIN(PYBIND11_NAMESPACE)
PYBIND11_NAMESPACE_BEGIN(detail)

template <> struct type_caster<nlohmann::json> {
  public:
    PYBIND11_TYPE_CASTER(nlohmann::json, const_name("json"));

    bool load(handle src, bool convert)
    {
        if (!src) {
            return false;
        }
        if (!convert && !py::isinstance<py::str>(src)) {
            return false;
        }
        try {
            value = pyjson::from_python(py::reinterpret_borrow<py::object>(src));
            return true;
        } catch (const std::exception &e) {
            pybind11::set_error(PyExc_ValueError, e.what());
            return false;
        }
    }

    static handle cast(const nlohmann::json &src, return_value_policy /* policy */,
                       handle /* parent */)
    {
        return pyjson::to_python(src).release();
    }
};

PYBIND11_NAMESPACE_END(detail)
PYBIND11_NAMESPACE_END(PYBIND11_NAMESPACE)
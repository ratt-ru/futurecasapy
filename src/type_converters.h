#include <iostream>

#include <casacore/casa/Containers/Record.h>

#include <pybind11/pybind11.h>
#include <pybind11/complex.h>

#if PY_MAJOR_VERSION <= 2
#error Only Python 3 supported
#endif

namespace py = pybind11;

namespace pybind11 {
namespace detail {

template <> struct type_caster<casacore::String> {
public:
    PYBIND11_TYPE_CASTER(casacore::String, _("casacore::String"));
    bool load(handle src, bool convert)
    {
        if(!src)
            { return false; }

        bool is_unicode_str = PyUnicode_Check(src.ptr());

        if(!convert && !is_unicode_str)
            { return false; }

        value = src.cast<std::string>();

        // // Owned reference
        // PyObject * temp_bytes = PyUnicode_AsEncodedString(src.ptr(), "UTF-8", "strict");

        // std::cout << "casacore::String " << src.ptr() << " " << temp_bytes << std::endl;

        // if(temp_bytes == nullptr)
        //     { return false; }

        // // Take over lifetime management of temp_bytes
        // auto str = py::reinterpret_steal<py::str>(temp_bytes);
        // value = str.cast<std::string>();
        return true;
    }

    static handle cast(casacore::String src, return_value_policy /* policy */, handle /* parent */)
        { return py::str(src); }
};

template <> struct type_caster<casacore::ValueHolder> {
public:
    PYBIND11_TYPE_CASTER(casacore::ValueHolder, _("casacore::ValueHolder"));

    bool load(handle src, bool convert) {
        if(!src)
            { return false; }

        bool is_none = src.is_none();
        bool is_bool = PyBool_Check(src.ptr());
        bool is_long = PyLong_Check(src.ptr());
        // bool is_int = PyInt_Check(o);
        bool is_double = PyFloat_Check(src.ptr());
        bool is_complex = PyComplex_Check(src.ptr());
        bool is_string = PyUnicode_Check(src.ptr());
        bool is_dict = PyDict_Check(src.ptr());
        // bool is_array = PycArrayCheck(o);

        if(!convert) {
            return is_none || is_bool || is_long || is_double || is_complex || is_string || is_dict;
        }

        if(is_none) {
            std::cout << "None ValueHolder" << std::endl;
            value = casacore::ValueHolder(0, true);
            return true;
        } else if(is_bool) {
            std::cout << "bool ValueHolder" << std::endl;
            value = casacore::ValueHolder(src.cast<bool>());
            return true;
        } else if(is_long) {
            std::cout << "long ValueHolder" << std::endl;
            value = casacore::ValueHolder(casacore::Int64(src.cast<long>()));
            return true;
        } else if(is_double) {
            std::cout << "double ValueHolder" << std::endl;
            value = casacore::ValueHolder(src.cast<double>());
            return true;
        } else if(is_complex) {
            std::cout << "complex ValueHolder" << std::endl;
            value = casacore::ValueHolder(src.cast<std::complex<double>>());
            return true;
        } else if(is_string) {
            std::cout << "string ValueHolder: " << src.cast<std::string>() << std::endl;
            value = casacore::ValueHolder(src.cast<std::string>());
            return true;
        } else if(is_dict) {
            std::cout << "dict ValueHolder" << std::endl;
            value = casacore::ValueHolder(src.cast<casacore::Record>());
            return true;
        }

        throw std::runtime_error("Unhandled ValueError Type");

        return false;
    }

    static handle cast(casacore::ValueHolder src, return_value_policy /* policy */, handle /* parent */) {
        switch(src.dataType()) {
            case casacore::TpBool:
                return py::cast<bool>(src.asBool());
                // return py::cast(src.asBool());
                break;
            case casacore::TpChar:
	        case casacore::TpShort:
            case casacore::TpInt:
                return py::cast<int>(src.asInt());
                // return py::cast(src.asInt());
                break;
            case casacore::TpInt64:
                throw std::runtime_error("Int64 not yet implemented");
                return py::cast<casacore::Int64>(src.asInt64());
                // return py::cast(src.asInt64());
                break;
            case casacore::TpUChar:
            case casacore::TpUShort:
            case casacore::TpUInt:
                return py::cast<unsigned int>(src.asuInt());
                // return py::cast(src.asuInt());
                break;
            case casacore::TpFloat:
            case casacore::TpDouble:
               return py::cast<double>(src.asDouble());
                // return py::cast(src.asDouble());
                break;
            case casacore::TpComplex:
            case casacore::TpDComplex:
               return py::cast<std::complex<double>>(src.asDComplex());
                // return py::cast(src.asDComplex());
                break;
            case casacore::TpString:
                return py::cast<const std::string &>(src.asString());
                // return py::str(src.asString());
                break;
	        case casacore::TpRecord:
                return py::cast(src.asRecord());
                break;
            // case casacore::TpLDouble:
	        case casacore::TpArrayBool:
            case casacore::TpArrayChar:
            case casacore::TpArrayUChar:
	        case casacore::TpArrayShort:
            case casacore::TpArrayUShort:
            case casacore::TpArrayInt:
            case casacore::TpArrayUInt:
            case casacore::TpArrayInt64:
            case casacore::TpArrayFloat:
            case casacore::TpArrayDouble:
            // case casacore::TpArrayLDouble:
            case casacore::TpArrayComplex:
            case casacore::TpArrayDComplex:
            case casacore::TpArrayString:
                throw std::runtime_error("Arrays not implemented");
                break;
            default:
                throw std::runtime_error("Unhandled type");
                break;
        }
    }
};

template <> struct type_caster<casacore::Record> {
public:
    /**
     * This macro establishes the name 'casacore::Record' in
     * function signatures and declares a local variable
     * 'value' of type casacore::Record
     */
    PYBIND11_TYPE_CASTER(casacore::Record, _("casacore::Record"));

    /**
     * Conversion part 1 (Python->C++): convert a PyObject into a casacore::Record
     * instance or return false upon failure. The second argument
     * indicates whether implicit conversions should be applied.
     */
    bool load(handle src, bool convert) {
        if(!src)
            { return false; }

        bool is_dict = PyDict_Check(src.ptr());

        if(!convert && !is_dict)
            { return false; }

        if(!is_dict)
            { return false; }

        //py::dict d = py::cast<py::dict>(src);
        auto d = py::dict(py::object(src, true));

        for (auto & item: d)
        {
            if(!PyUnicode_Check(item.first.ptr())) {
                throw std::runtime_error("key must be a string");
            }

            value.defineFromValueHolder(item.first.cast<std::string>(),
                                        item.second.cast<casacore::ValueHolder>());
        }

        return true;
    }

    /**
     * Conversion part 2 (C++ -> Python): convert an casacore::Record instance into
     * a Python object. The second and third arguments are used to
     * indicate the return value policy and parent object (for
     * ``return_value_policy::reference_internal``) and are generally
     * ignored by implicit casters.
     */
    static handle cast(casacore::Record src, return_value_policy /* policy */, handle /* parent */)
    {
        return py::dict();
        // py::dict d;
        // casacore::uInt nf = src.nfields();

        // for(casacore::uInt f=0; f < nf; ++f) {
        //     d[py::str(src.name(f))] = py::cast<casacore::ValueHolder>(src.asValueHolder(f));
        // }

        // return d;
    }
};

} // namespace detail
} // namespace pybind11
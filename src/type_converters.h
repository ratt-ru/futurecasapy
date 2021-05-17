#include <iostream>
#include <memory>
#include <type_traits>

#include <casacore/casa/Containers/Record.h>

#include <pybind11/pybind11.h>
#include <pybind11/complex.h>
#include <pybind11/numpy.h>

#if PY_MAJOR_VERSION <= 2
#error Only Python 3 supported
#endif

namespace py = pybind11;


class ManagedValueHolder
{
private:
    casacore::ValueHolder vh;
    py::handle h;


public:
    ManagedValueHolder() :
        vh(),
        h()
        {};

    ManagedValueHolder(casacore::ValueHolder vh, py::handle handle) :
        vh(vh),
        h(handle)
        {}

    ManagedValueHolder(ManagedValueHolder & other) :
        vh(other.vh),
        h(other.h)
        {}

    ManagedValueHolder(ManagedValueHolder && other) :
        vh(std::move(other.vh)),
        h(std::move(other.h))
        {}

    ManagedValueHolder & operator=(ManagedValueHolder other)
    {
        std::swap(vh, other.vh);
        std::swap(h, other.h);
        return *this;
    }
};


template <typename T> class ManagedArray
{
private:
    using A = casacore::Array<T>;
    using PTR = std::shared_ptr<A>;

private:
    bool owned_;
    PTR array_;
public:
    explicit ManagedArray(const casacore::IPosition & shape, T * buffer, bool owned=false):
        owned_(owned),
        array_(std::make_shared<A>(shape, buffer, casacore::SHARE))
        {
            std::cout << "ManagedArray(shape, buffer, owned): " << array_.get() << std::endl;
        }

    ManagedArray(const casacore::Array<T> & array, bool owned=true) :
        owned_(owned),
        array_(std::make_shared<A>(array))
        {
            std::cout << "ManagedArray(array, owned): " << array_.get() << std::endl;

        }

    ManagedArray() :
        owned_(false),
        array_(nullptr)
        {
            std::cout << "ManagedArray(): " << array_.get() << std::endl;
        }

    ManagedArray(const ManagedArray & other) :
        owned_(other.owned_),
        array_(other.array_)
        {
            std::cout << "ManagedArray(other&): " << array_.get() << std::endl;

        }

    ManagedArray(ManagedArray && other) :
        owned_(other.owned_),
        array_(std::move(other.array_)) {
            std::cout << "ManagedArray(other&&): " << array_.get() << std::endl;

        }

    ManagedArray & operator=(ManagedArray other)
    {
        std::cout << "ManagedArray::operator=(other): " << array_.get() << std::endl;
        std::swap(this->owned_, other.owned_);
        std::swap(this->array_, other.array_);
        return *this;
    }

public:
    bool owned(void) const
        { return this->owned_; }

    const PTR ptr(void) const
        { return this->array_; }

    const casacore::Array<T> & array(void) const
        { return *this->array_.get(); }

};

namespace pybind11 {
namespace detail {

using NPY_TYPE = py::detail::npy_api::constants;

template <typename C>
struct CasaTypeTraits
{
    static const bool valid = false;
    using CASA_TYPE = C;
    using CPP_TYPE = void;
    static const NPY_TYPE npy_type = py::detail::npy_api::NPY_BOOL_;
    static const bool equivalent = false;
};

template <> struct CasaTypeTraits<casacore::Bool> {
    static const bool valid = true;
    using CASA_TYPE = casacore::Bool;
    using CPP_TYPE = bool;
    static const NPY_TYPE npy_type = py::detail::npy_api::NPY_BOOL_;
    static const bool equivalent = std::is_same<CASA_TYPE, CPP_TYPE>::value;
};


template <> struct CasaTypeTraits<casacore::String> {
    static const bool valid = true;
    using CASA_TYPE = casacore::String;
    using CPP_TYPE = std::string;
    static const NPY_TYPE npy_type = py::detail::npy_api::NPY_STRING_;
    static const bool equivalent = std::is_same<CASA_TYPE, CPP_TYPE>::value;
};


template <> struct CasaTypeTraits<casacore::Complex> {
    static const bool valid = true;
    using CASA_TYPE = casacore::Complex;
    using CPP_TYPE = std::complex<float>;
    static const NPY_TYPE npy_type = py::detail::npy_api::NPY_CFLOAT_;
    static const bool equivalent = std::is_same<CASA_TYPE, CPP_TYPE>::value;
};

template <> struct CasaTypeTraits<casacore::DComplex> {
    static const bool valid = true;
    using CASA_TYPE = casacore::DComplex;
    using CPP_TYPE = std::complex<double>;
    static const NPY_TYPE npy_type = py::detail::npy_api::NPY_CDOUBLE_;
    static const bool equivalent = std::is_same<CASA_TYPE, CPP_TYPE>::value;
};


template <> struct type_caster<casacore::Record>;
template <> struct type_caster<casacore::ValueHolder>;

template <> struct type_caster<casacore::String> {
public:
    using CT = CasaTypeTraits<casacore::String>;

    PYBIND11_TYPE_CASTER(casacore::String, _("casacore::String"));
    bool load(handle src, bool convert)
    {
        std::cout << "CASA and CPP types equivalent " << std::endl;
        value = std::move(src.cast<CT::CPP_TYPE>());
        return true;
    }

    static handle cast(casacore::String src, return_value_policy /* policy */, handle /* parent */)
    {
        return py::str(CT::CPP_TYPE(src)).release();
    }
};


// template <typename T> struct type_caster<casacore::Array<T>> {
// public:
//     PYBIND11_TYPE_CASTER(casacore::Array<T>, _("casacore::Array<T>"));
//     bool load(handle src, bool convert)
//     {

//         if(!src)
//             { return false; }

//         const auto &numpy_api = detail::npy_api::get();
//         if(!numpy_api.PyArray_Check_(src.ptr()))
//             { return false; };

//         auto array = py::cast<py::array>(src.ptr());
//         auto ndim = array.ndim();
//         auto flags = array.flags();
//         const auto & shape = array.shape();

//         std::cout << "Casting python to casacore<Array<T>> "
//                   << shape << std::endl;

//         // Copy the array if its not contiguous or aligned
//         // TODO(sjperkins)
//         // Should also copy if endianess differs
//         auto is_contiguous = ((flags & numpy_api.NPY_ARRAY_C_CONTIGUOUS_) ||
//                               (flags & numpy_api.NPY_ARRAY_F_CONTIGUOUS_));
//         auto is_aligned = (flags & numpy_api.NPY_ARRAY_ALIGNED_);

//         if(!is_contiguous || !is_aligned) {
//             array = array.attr("copy")();
//         }

//         // Swap axes, CASA order is row minor,
//         // Numpy is row major.
//         casacore::IPosition casa_shape(1, 1);

//         if(ndim > 0)
//         {
//             casa_shape.resize(ndim);
//             for(int d=0; d < ndim; ++d)
//                 { casa_shape[d] = shape[ndim-d-1]; }
//         }

//         if(casa_shape.product() > 0)
//         {
//             flags = array.flags();

//             is_contiguous = ((flags & numpy_api.NPY_ARRAY_C_CONTIGUOUS_) ||
//                              (flags & numpy_api.NPY_ARRAY_F_CONTIGUOUS_));

//             if(!is_contiguous)
//                 { throw std::runtime_error("Non-contiguous array"); }
//         }

//         auto tmp = ManagedArray<T>(casa_shape, (T *) array.data());

//         value = casacore::Array<T>(casa_shape, (T *) array.data(), casacore::SHARE);

//         return true;
//     }

//     static handle cast(casacore::Array<T> src, return_value_policy /* policy */, handle /* parent */)
//     {
//         using CT = CasaTypeTraits<T>;

//         auto ndim = src.ndim();
//         std::vector<py::ssize_t> shape;
//         auto & casa_shape = src.shape();

//         for(decltype(ndim) d=0; d < ndim; ++d)
//             { shape.push_back(casa_shape[ndim-d-1]); }

//         casacore::Array<T> * saved = new casacore::Array<T>(src);
//         bool delete_storage = false;
//         auto * storage = (typename CT::CPP_TYPE *) saved->getStorage(delete_storage);
//         auto capsule = py::capsule(saved, [](void * p) {
//             // std::cout << "Deleting " << p << std::endl;
//             delete reinterpret_cast<casacore::Array<T> *>(p);
//         });

//         if(delete_storage) {
//             throw std::runtime_error("should delete storage!");
//         }

//         return py::array(shape, storage, capsule);
//     }
// };

template <typename T> struct type_caster<ManagedArray<T>> {
public:
    PYBIND11_TYPE_CASTER(ManagedArray<T>, _("ManagedArray<T>"));
    bool load(handle src, bool convert)
    {
        if(!src)
            { return false; }

        const auto &numpy_api = detail::npy_api::get();
        if(!numpy_api.PyArray_Check_(src.ptr()))
            { return false; };

        auto array = py::cast<py::array>(src.ptr());
        auto ndim = array.ndim();
        auto flags = array.flags();
        const auto & shape = array.shape();

        // Copy the array if its not contiguous or aligned
        // TODO(sjperkins)
        // Should also copy if endianess differs
        auto is_contiguous = ((flags & numpy_api.NPY_ARRAY_C_CONTIGUOUS_) ||
                              (flags & numpy_api.NPY_ARRAY_F_CONTIGUOUS_));
        auto is_aligned = (flags & numpy_api.NPY_ARRAY_ALIGNED_);

        if(!is_contiguous || !is_aligned) {
            array = array.attr("copy")();
        }

        // Swap axes, CASA order is row minor,
        // Numpy is row major.
        casacore::IPosition casa_shape(1, 1);

        if(ndim > 0)
        {
            casa_shape.resize(ndim);
            for(int d=0; d < ndim; ++d)
                { casa_shape[d] = shape[ndim-d-1]; }
        }

        // std::cout << "Casting python to casacore<Array<T>> "
        //           << casa_shape << std::endl;


        if(casa_shape.product() > 0)
        {
            flags = array.flags();

            is_contiguous = ((flags & numpy_api.NPY_ARRAY_C_CONTIGUOUS_) ||
                             (flags & numpy_api.NPY_ARRAY_F_CONTIGUOUS_));

            if(!is_contiguous)
                { throw std::runtime_error("Non-contiguous array"); }
        }

        // Create a ManagedArray, Python owns the storage, not C++
        value = ManagedArray<T>(casa_shape, (T *) array.data(), false);

        return true;
    }

    static handle cast(ManagedArray<T> src, return_value_policy /* policy */, handle /* parent */)
    {
        using CT = CasaTypeTraits<T>;

        auto ndim = src.array().ndim();
        std::vector<py::ssize_t> shape;
        auto & casa_shape = src.array().shape();

        for(decltype(ndim) d=0; d < ndim; ++d)
            { shape.push_back(casa_shape[ndim-d-1]); }

        if(src.owned())
        {
            std::cout << "Owned storage" << std::endl;
            // C++ owns the storage, so the buffers must live as long as there's
            // a Python object pointing at it
            bool delete_storage = false;
            auto * saved = new ManagedArray<T>(src);
            auto * storage = (typename CT::CPP_TYPE *) saved->array().getStorage(delete_storage);
            auto capsule = py::capsule(saved, [](void * p)
            {
                auto ptr = reinterpret_cast<decltype(saved)>(p);
                std::cout << "Removing owned storage " << ptr->ptr().get() << std::endl;

                // delete reinterpret_cast<ManagedArray<T> *>(p);
            });

            if(delete_storage)
                { throw std::runtime_error("Unhandled storage deletion case!"); }

            return py::array(shape, storage, capsule);
        } else {
            // C++ does not own the storage, so create a noop capsule to ensure that
            // the underlying base isn't released
            // TODO(sjperkins)
            // there's probably a way to set the base to null to avoid the capsule
            std::cout << "UnOwned storage" << std::endl;
            bool delete_storage = false;
            auto * storage = (typename CT::CPP_TYPE *) src.array().getStorage(delete_storage);

            if(delete_storage)
                { throw std::runtime_error("Unhandled storage deletion case!"); }

            auto capsule = py::capsule((void *) 12345, [](void * p) {
                std::cout << "Empty capsule" << std::endl;
            });

            return py::array(shape, storage, capsule);
        }
    }
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

        const auto &numpy_api = detail::npy_api::get();
        bool is_array = numpy_api.PyArray_Check_(src.ptr());

        if(is_none) {
            value = casacore::ValueHolder(0, true);
            return true;
        } else if(is_bool) {
            value = casacore::ValueHolder(src.cast<bool>());
            return true;
        } else if(is_long) {
            value = casacore::ValueHolder(casacore::Int64(src.cast<int64_t>()));
            return true;
        } else if(is_double) {
            value = casacore::ValueHolder(src.cast<double>());
            return true;
        } else if(is_complex) {
            value = casacore::ValueHolder(src.cast<std::complex<double>>());
            return true;
        } else if(is_string) {
            value = casacore::ValueHolder(src.cast<casacore::String>());
            return true;
        } else if(is_array) {
            auto array_proxy = py::detail::array_proxy(src.ptr());
            auto desc = py::detail::array_descriptor_proxy(array_proxy->descr);

            std::cout << "Found an array" << std::endl;
            std::cout << "Kind: " << desc->kind
                      << " Type: " << desc->type
                      << " TypeNum: " << desc->type_num
                      << " ByteOrder: " << int(desc->byteorder) << std::endl;


            switch(desc->type_num)
            {
                case numpy_api.NPY_CFLOAT_:
                {
                    auto array = src.cast<ManagedArray<casacore::Complex>>().array();
                    value = casacore::ValueHolder(array);
                    return true;
                }
                case numpy_api.NPY_CDOUBLE_:
                {
                    auto array = src.cast<ManagedArray<casacore::DComplex>>().array();
                    value = casacore::ValueHolder(array);
                    return true;
                }
                // case numpy_api.NPY_BOOL_:
                // case numpy_api.NPY_BYTE_:
                // case numpy_api.NPY_UBYTE_:
                // case numpy_api.NPY_SHORT_:
                // case numpy_api.NPY_USHORT_:
                // case numpy_api.NPY_INT_:
                // case numpy_api.NPY_UINT_:
                // case numpy_api.NPY_LONG_:
                // case numpy_api.NPY_ULONG_:
                // case numpy_api.NPY_LONGLONG_:
                // case numpy_api.NPY_ULONGLONG_:
                // case numpy_api.NPY_FLOAT_:
                // case numpy_api.NPY_DOUBLE_:
                // case numpy_api.NPY_LONGDOUBLE_:
                // case numpy_api.NPY_CFLOAT_:
                // case numpy_api.NPY_CDOUBLE_:
                // case numpy_api.NPY_CLONGDOUBLE_:
                // case numpy_api.NPY_OBJECT_:
                // case numpy_api.NPY_STRING_:
                // case numpy_api.NPY_UNICODE_:
                // case numpy_api.NPY_VOID_:
                // Platform-dependent normalization
                // case numpy_api.NPY_INT8_:
                // case numpy_api.NPY_UINT8_:
                // case numpy_api.NPY_INT16_;
                // case numpy_api.NPY_UINT16_:
                // case numpy_api.NPY_INT32_:
                // case numpy_api.NPY_UINT32_:
                // case numpy_api.NPY_INT64_:
                // case numpy_api.NPY_UINT64_:
                default:
                    break;
            }

            return false;
        } else if(is_dict) {
            value = casacore::ValueHolder(src.cast<casacore::Record>());
            return true;
        }

        throw std::runtime_error("Unhandled ValueError Type");

        return false;
    }

    static handle cast(casacore::ValueHolder src, return_value_policy /* policy */, handle /* parent */) {
        std::cout << "Casting ValueHolder " << src << " to Python" << std::endl;

        switch(src.dataType()) {
            case casacore::TpBool:
                return py::cast<bool>(src.asBool()).release();
                break;
            case casacore::TpChar:
	        case casacore::TpShort:
            case casacore::TpInt:
                return py::cast<int32_t>(src.asInt()).release();
                break;
            case casacore::TpInt64:
                return py::cast<int64_t>(src.asInt64()).release();
                // return py::cast(src.asInt64());
                break;
            case casacore::TpUChar:
            case casacore::TpUShort:
            case casacore::TpUInt:
                return py::cast<uint32_t>(src.asuInt()).release();
                // return py::cast(src.asuInt());
                break;
            case casacore::TpFloat:
            case casacore::TpDouble:
               return py::cast<double>(src.asDouble()).release();
                // return py::cast(src.asDouble());
                break;
            case casacore::TpComplex:
            case casacore::TpDComplex:
               return py::cast<std::complex<double>>(src.asDComplex()).release();
                // return py::cast(src.asDComplex());
                break;
            case casacore::TpString:
                {
                    std::string s = src.asString();
                    return py::cast<std::string>(std::move(s)).release();
                    break;
                }
	        case casacore::TpRecord:
                {
                    casacore::Record r = src.asRecord();
                    return py::cast<casacore::Record>(std::move(r)).release();
                    break;
                }
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
                {
                    using CT = ManagedArray<casacore::Complex>;
                    return py::cast<CT>(std::move(CT(src.asArrayComplex(), false))).release();
                }
            case casacore::TpArrayDComplex:
                {
                    using CT = ManagedArray<casacore::DComplex>;
                    return py::cast<CT>(std::move(CT(src.asArrayDComplex(), false))).release();
                }
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
    PYBIND11_TYPE_CASTER(casacore::Record, _("casacore::Record"));

    bool load(handle src, bool convert){
        if(!src || !PyDict_Check(src.ptr()))
            { return false; }

        py::dict d = py::cast<py::dict>(src);

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

    static handle cast(casacore::Record src, return_value_policy /* policy */, handle /* parent */)
    {
        py::dict d;
        casacore::uInt nf = src.nfields();

        for(casacore::uInt f=0; f < nf; ++f) {
            // d[py::str(src.name(f))] = py::cast<casacore::ValueHolder>(src.asValueHolder(f));
            std::cout << src.name(f) << " " << src.asValueHolder(f) << std::endl;
            d[py::str(src.name(f))] = src.asValueHolder(f);
            // d[py::str("k") + py::str(py::cast<int>(nf))] = py::str("Hello");
        }

        return d.release();
    }
};

} // namespace detail
} // namespace pybind11
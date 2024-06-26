#pragma once

#include "cast.h"

namespace pybind11
{
    template <typename T>
    T& _builtin_cast(handle obj)
    {
        static_assert(!std::is_reference_v<T>, "T must not be a reference type.");
        return ((pkpy::Py_<T>*)(obj.ptr()))->_value;
    }

    template <typename T>
    inline bool isinstance(handle obj)
    {
        pkpy::Type cls = _builtin_cast<pkpy::Type>(type::handle_of<T>().ptr());
        return vm->isinstance(obj.ptr(), cls);
    }

    template <>
    inline bool isinstance<handle>(handle) = delete;

    inline bool isinstance(handle obj, handle type)
    {
        return vm->isinstance(obj.ptr(), _builtin_cast<pkpy::Type>(type));
    }

    inline bool hasattr(handle obj, handle name)
    {
        return obj.ptr()->attr().try_get(_builtin_cast<pkpy::Str>(name)) != nullptr;
    }

    inline bool hasattr(handle obj, const char* name)
    {
        return obj.ptr()->attr().try_get(name) != nullptr;
    }

    inline void delattr(handle obj, handle name)
    {
        bool result = obj.ptr()->attr().del(_builtin_cast<pkpy::Str>(name));
        // TODO: delete not existed attribute
    }

    inline void delattr(handle obj, const char* name)
    {
        obj.ptr()->attr().del(name);
        // TODO: delete not existed attribute
    }

    inline object getattr(handle obj, handle name)
    {
        // TODO: get not existed attribute
        handle attr = vm->getattr(obj.ptr(), _builtin_cast<pkpy::Str>(name));
        return reinterpret_borrow<object>(attr);
    }

    inline object getattr(handle obj, const char* name)
    {
        // TODO: get not existed attribute
        handle result = vm->getattr(obj.ptr(), name);
        return reinterpret_borrow<object>(result);
    }

    inline object getattr(handle obj, handle name, handle default_);

    inline object getattr(handle obj, const char* name, handle default_);

    inline void setattr(handle obj, handle name, handle value)
    {
        // TODO: set not existed attribute
        vm->setattr(obj.ptr(), _builtin_cast<pkpy::Str>(name), value.ptr());
    }

    inline void setattr(handle obj, const char* name, handle value)
    {
        // TODO: set not existed attribute
        vm->setattr(obj.ptr(), name, value.ptr());
    }

    inline int64_t hash(handle obj) { return vm->py_hash(obj.ptr()); }

    template <typename T>
    handle _cast(T&& value,
                 return_value_policy policy = return_value_policy::automatic_reference,
                 handle parent = handle())
    {
        using U = std::remove_pointer_t<std::remove_cv_t<std::remove_reference_t<T>>>;
        return type_caster<U>::cast(std::forward<T>(value), policy, parent);
    }

    template <typename T>
    object cast(T&& value,
                return_value_policy policy = return_value_policy::automatic_reference,
                handle parent = handle())
    {
        return reinterpret_borrow<object>(_cast(std::forward<T>(value), policy, parent));
    }

    template <typename T>
    T cast(handle obj, bool convert = false)
    {
        using Caster =
            type_caster<std::remove_pointer_t<std::remove_cv_t<std::remove_reference_t<T>>>>;
        Caster caster;

        if(caster.load(obj, convert))
        {
            if constexpr(std::is_rvalue_reference_v<T>)
            {
                return std::move(value_of_caster(caster));
            }
            else
            {
                return value_of_caster(caster);
            }
        }
        throw std::runtime_error("Unable to cast Python instance to C++ type");
    }

}  // namespace pybind11


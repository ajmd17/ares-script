#ifndef DYNAMIC_H
#define DYNAMIC_H

#include <type_traits>
#include <typeinfo>
#include <memory>
#include <utility>
#include <iostream>

namespace avm {
class Dynamic {
public:
    Dynamic()
    {
        ptr = nullptr;
        holder = nullptr;
    }

    template <typename T>
    explicit Dynamic(T value)
    {
        Assign(value);
    }

    explicit Dynamic(const Dynamic &other)
    {
        auto copy = other.holder->Clone();
        holder = std::move(copy.first);
        ptr = copy.second;
    }

    Dynamic &operator=(const Dynamic &other)
    {
        auto copy = other.holder->Clone();
        holder = std::move(copy.first);
        ptr = copy.second;
        return *this;
    }

    template <typename T>
    bool operator==(T t)
    {
        return Compatible<T>() && Get<T>() == t;
    }

    bool operator==(const Dynamic &other)
    {
        return holder->Compare(other.holder.get());
    }

    template <typename T>
    inline bool Compatible() const
    {
        typedef typename std::decay<T>::type U;
        return holder->TypeInfo() == typeid(U);
    }

    template <typename T>
    void Assign(T value)
    {
        typedef typename std::decay<T>::type U;
        std::unique_ptr<DerivedHolder<U>> impl(new DerivedHolder<U>(value));
        ptr = reinterpret_cast<void*>(&impl->value);
        holder = std::move(impl);
    }

    template <typename T>
    T &Get() const
    {
        typedef typename std::decay<T>::type U;
        if (!Compatible<T>()) {
            throw std::bad_cast();
        }
        return *reinterpret_cast<U*>(ptr);
    }

    bool IsNull() const
    {
        return ptr == nullptr || holder == nullptr || TypeInfo() == typeid(nullptr);
    }

    const std::type_info &TypeInfo() const
    {
        return holder->TypeInfo();
    }

private:
    struct BaseHolder {
        virtual ~BaseHolder() {}
        virtual std::pair<std::unique_ptr<BaseHolder>, void*> Clone() = 0;
        virtual bool Compare(const BaseHolder *) const = 0;
        virtual const std::type_info &TypeInfo() const = 0;
    };

    template <typename T>
    struct DerivedHolder : public BaseHolder {
        explicit DerivedHolder(T value) : value(value)
        {
        }

        std::pair<std::unique_ptr<BaseHolder>, void*> Clone()
        {
            std::pair<std::unique_ptr<DerivedHolder<T>>, void*> result;
            result.first = std::unique_ptr<DerivedHolder<T>>(new DerivedHolder<T>(value));
            result.second = reinterpret_cast<void*>(&(result.first->value));

            return std::move(result);
        }

        bool Compare(const BaseHolder *other) const
        {
            if (!other) return false;
            auto other_holder = dynamic_cast<const DerivedHolder<T>*>(other);
            return other_holder != nullptr && other_holder->value == value;
        }

        const std::type_info &TypeInfo() const
        {
            return typeid(value);
        }

        T value;
    };

    std::unique_ptr<BaseHolder> holder;
    void *ptr;
};
} // namespace avm

#endif
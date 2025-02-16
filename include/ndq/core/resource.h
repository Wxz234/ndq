#pragma once

#include <compare>
#include <type_traits>

namespace ndq
{
    class IRefCounted
    {
    public:
        virtual unsigned long AddRef() = 0;
        virtual unsigned long Release() = 0;
    };

    template <typename T>
    requires std::is_base_of_v<IRefCounted, T>
    class TRefCountPtr
    {
    public:
        constexpr TRefCountPtr(decltype(nullptr) ptr = nullptr) noexcept : ptr_(ptr) {}
        TRefCountPtr(T* other) noexcept : ptr_(other)
        {
            InternalAddRef();
        }

        TRefCountPtr(const TRefCountPtr& other) noexcept : ptr_(other.ptr_)
        {
            InternalAddRef();
        }

        TRefCountPtr(TRefCountPtr&& other) noexcept : ptr_(nullptr)
        {
            Swap(other);
        }

        ~TRefCountPtr() noexcept
        {
            InternalRelease();
        }

        TRefCountPtr& operator=(decltype(nullptr)) noexcept
        {
            InternalRelease();
            return *this;
        }

        TRefCountPtr& operator=(T* other) noexcept
        {
            if (ptr_ != other)
            {
                TRefCountPtr(other).Swap(*this);
            }
            return *this;
        }

        TRefCountPtr& operator=(const TRefCountPtr& other) noexcept
        {
            if (ptr_ != other.ptr_)
            {
                TRefCountPtr(other).Swap(*this);
            }
            return *this;
        }

        TRefCountPtr& operator=(TRefCountPtr&& other) noexcept
        {
            TRefCountPtr(static_cast<TRefCountPtr&&>(other)).Swap(*this);
            return *this;
        }

        explicit operator bool() const noexcept
        {
            return Get() != nullptr;
        }

        T* Get() const noexcept
        {
            return ptr_;
        }

        T* operator->() const noexcept
        {
            return ptr_;
        }

        T* Detach() noexcept
        {
            T* ptr = ptr_;
            ptr_ = nullptr;
            return ptr;
        }

        T* const* GetAddressOf() const noexcept
        {
            return &ptr_;
        }

        T** GetAddressOf() noexcept
        {
            return &ptr_;
        }

        T** ReleaseAndGetAddressOf() noexcept
        {
            InternalRelease();
            return &ptr_;
        }

        void Swap(TRefCountPtr&& r) noexcept
        {
            T* tmp = ptr_;
            ptr_ = r.ptr_;
            r.ptr_ = tmp;
        }

        void Swap(TRefCountPtr& r) noexcept
        {
            T* tmp = ptr_;
            ptr_ = r.ptr_;
            r.ptr_ = tmp;
        }

        unsigned long Reset()
        {
            return InternalRelease();
        }

    private:
        T* ptr_;

        void InternalAddRef() const noexcept
        {
            if (ptr_ != nullptr)
            {
                ptr_->AddRef();
            }
        }

        unsigned long InternalRelease() noexcept
        {
            unsigned long ref = 0;
            T* temp = ptr_;

            if (temp != nullptr)
            {
                ptr_ = nullptr;
                ref = temp->Release();
            }

            return ref;
        }
    };

    template<typename T>
    bool operator==(const TRefCountPtr<T>& a, const TRefCountPtr<T>& b) noexcept
    {
        return a.Get() == b.Get();
    }

    template<typename T>
    bool operator==(const TRefCountPtr<T>& a, decltype(nullptr)) noexcept
    {
        return a.Get() == nullptr;
    }

    template<typename T>
    std::strong_ordering operator<=>(const TRefCountPtr<T>& a, const TRefCountPtr<T>& b) noexcept
    {
        return a.Get() <=> b.Get();
    }

    template<typename T>
    std::strong_ordering operator<=>(const TRefCountPtr<T>& a, decltype(nullptr)) noexcept
    {
        return a.Get() <=> static_cast<T*>(nullptr);
    }
}
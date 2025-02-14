#pragma once

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
    concept TRefCountPtrConcept = !std::is_reference_v<T> && !std::is_const_v<T> && std::is_base_of_v<IRefCounted, T>;

    template <TRefCountPtrConcept T>
    class TRefCountPtr
    {
    public:
        constexpr TRefCountPtr() noexcept : ptr_(nullptr) {}
        constexpr TRefCountPtr(decltype(nullptr)) noexcept : ptr_(nullptr) {}
        TRefCountPtr(T* ptr) : ptr_(ptr)
        {
            InternalAddRef();
        }

        TRefCountPtr(const TRefCountPtr& other) noexcept : ptr_(other.ptr_)
        {
            InternalAddRef();
        }

        TRefCountPtr(TRefCountPtr&& other) noexcept : ptr_(nullptr)
        {
            if (this != reinterpret_cast<TRefCountPtr*>(&reinterpret_cast<unsigned char&>(other)))
            {
                Swap(other);
            }
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

        [[nodiscard]] T* Get() const noexcept
        {
            return ptr_;
        }

        T* operator->() const noexcept
        {
            return ptr_;
        }

        T** operator&()
        {
            InternalRelease();
            return &ptr_;
        }

        explicit operator bool() const noexcept
        {
            return Get() != nullptr;
        }

        [[nodiscard]] T* const* GetAddressOf() const noexcept
        {
            return &ptr_;
        }

        [[nodiscard]] T** GetAddressOf() noexcept
        {
            return &ptr_;
        }

        [[nodiscard]] T** ReleaseAndGetAddressOf() noexcept
        {
            InternalRelease();
            return &ptr_;
        }

        T* Detach() noexcept
        {
            T* ptr = ptr_;
            ptr_ = nullptr;
            return ptr;
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
}
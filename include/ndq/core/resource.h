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
    concept RefCountPtrConcept = !std::is_reference_v<T> && !std::is_const_v<T> && std::is_base_of_v<IRefCounted, T>;

    template <RefCountPtrConcept T>
    class RefCountPtr
    {
    public:
        constexpr RefCountPtr() noexcept : ptr_(nullptr) {}
        constexpr RefCountPtr(decltype(nullptr)) noexcept : ptr_(nullptr) {}
        RefCountPtr(T* ptr) : ptr_(ptr)
        {
            InternalAddRef();
        }

        RefCountPtr(const RefCountPtr& other) noexcept : ptr_(other.ptr_)
        {
            InternalAddRef();
        }

        RefCountPtr(RefCountPtr&& other) noexcept : ptr_(nullptr)
        {
            if (this != reinterpret_cast<RefCountPtr*>(&reinterpret_cast<unsigned char&>(other)))
            {
                Swap(other);
            }
        }

        ~RefCountPtr() noexcept
        {
            InternalRelease();
        }

        RefCountPtr& operator=(decltype(nullptr)) noexcept
        {
            InternalRelease();
            return *this;
        }

        RefCountPtr& operator=(T* other) noexcept
        {
            if (ptr_ != other)
            {
                RefCountPtr(other).Swap(*this);
            }
            return *this;
        }

        RefCountPtr& operator=(const RefCountPtr& other) noexcept
        {
            if (ptr_ != other.ptr_)
            {
                RefCountPtr(other).Swap(*this);
            }
            return *this;
        }

        RefCountPtr& operator=(RefCountPtr&& other) noexcept
        {
            RefCountPtr(static_cast<RefCountPtr&&>(other)).Swap(*this);
            return *this;
        }

        void Swap(RefCountPtr&& r) noexcept
        {
            T* tmp = ptr_;
            ptr_ = r.ptr_;
            r.ptr_ = tmp;
        }

        void Swap(RefCountPtr& r) noexcept
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
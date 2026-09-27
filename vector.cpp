// Task2 = Implement your own vector

#include <print>
#include <cstring>
#include <string>
#include <vector>
#include <type_traits>
#include <utility>
#include <new>
#include <cassert>


// TODO:
    // use std::move_if_noexcept
    // mark move and copy semantics 'noexcept' to complete the previous task
    // try catch possible exceptions to guarantee 'noexcept'
    // optional: add custom allocator support
    // optional: add iterator support


template <typename T>
class Vector {
public:
    Vector()
    : size_     {0}
    , capacity_ {0}
    , data_     {nullptr} {}

    Vector(size_t capacity)
    : size_     {0}
    , capacity_ {capacity}
    , data_     {static_cast<T*>(::operator new(sizeof(T) * capacity_, std::align_val_t{alignof(T)}))} {}

    Vector(const Vector& other)
    : size_     {other.size_}
    , capacity_ {other.capacity_}
    , data_     {static_cast<T*>(::operator new(sizeof(T) * capacity_, std::align_val_t{alignof(T)}))}
    {
        if constexpr (std::is_trivially_copyable_v<T>) {
            std::memcpy(data_, other.data_, sizeof(T) * size_);
        } else {
            for (size_t i = 0; i < size_; ++i) {
                new (data_ + i) T(other.data_[i]);
            }
        }
    }

    Vector& operator =(const Vector& other)
    {
        if (this == &other) { return *this; }

        T* tmp = nullptr;
        if (capacity_ < other.size_) {
            tmp = static_cast<T*>(::operator new(sizeof(T) * other.size_, std::align_val_t{alignof(T)}));
        }

        if constexpr (!std::is_trivially_destructible_v<T>) {
            for (size_t i = 0; i < size_; ++i) { data_[i].~T(); }
        }

        if (tmp != nullptr) {
            ::operator delete(data_, sizeof(T) * capacity_, std::align_val_t{alignof(T)});
            capacity_ = other.size_;
            data_ = tmp;
        }

        if constexpr (std::is_trivially_copyable_v<T>) {
            size_ = other.size_;
            std::memcpy(data_, other.data_, sizeof(T) * size_);
        } else {
            for (size_ = 0; size_ < other.size_; ++size_) {
                new (data_ + size_) T(other.data_[size_]);
            }
        }

        return *this;
    }

    Vector(Vector&& other) noexcept
    : size_     { other.size_ }
    , capacity_ { other.capacity_ }
    , data_     { std::move(other.data_) }
    {
        other.size_ = 0;
        other.capacity_ = 0;
        other.data_ = nullptr;
    }

    Vector& operator=(Vector&& other) noexcept
    {
        if (this == &other) { return *this; }

        if constexpr (!std::is_trivially_destructible_v<T>) {
            for (size_t i = 0; i < size_; ++i) { data_[i].~T(); }
        }
        ::operator delete(data_, std::align_val_t{alignof(T)});

        size_     = std::exchange(other.size_, 0);
        capacity_ = std::exchange(other.capacity_, 0);
        data_     = std::exchange(other.data_, nullptr);

        return *this;
    }

    ~Vector()
    {
        if (!data_) { return; }

        if (!std::is_trivially_destructible_v<T>) {
            for (size_t i = 0; i < size_; ++i) {
                data_[i].~T();
            }
        }

        ::operator delete(data_, sizeof(T) * capacity_, std::align_val_t{alignof(T)});
    }

    template <class... Arg>
    void emplace_back(Arg&&... args)
    {
        if (size_ == capacity_) { enlarge(); }
        new (data_ + size_) T(std::forward<Arg>(args)...);
        ++size_;
    }

    void push_back(const T& value)
    {
        emplace_back(value);
    }

    void push_back(T&& value)
    {
        emplace_back(std::move(value));
    }

    void pop_back() noexcept
    {
        assert(size_ != 0);

        if constexpr (!std::is_trivially_destructible_v<T>) {
            data_[size_ - 1].~T();
        }
        --size_;
    }

    size_t size() const noexcept
    {
        return size_;
    }

    size_t capacity() const noexcept
    {
        return capacity_;
    }

    T& operator[](size_t index) noexcept
    {
        return data_[index];
    }

    const T& operator[](size_t index) const noexcept
    {
        return data_[index];
    }

    T& at(size_t index)
    {
        assert(index < size_);
        return data_[index];
    }

    const T& at(size_t index) const
    {
        assert(index < size_);
        return data_[index];
    }

    T* data() noexcept
    {
        return data_;
    }

    const T* data() const noexcept
    {
        return data_;
    }

    T* begin() noexcept
    {
        return data_;
    }

    T* end() noexcept
    {
        return data_ + size_;
    }

    const T* cbegin() const noexcept
    {
        return data_;
    }

    const T* cend() const noexcept
    {
        return data_ + size_;
    }

    bool empty() const noexcept
    {
        return !size_;
    }

private:
    size_t size_;
    size_t capacity_;
    T* data_;

    void enlarge()
    {
        size_t new_capacity = capacity_ ? capacity_ * 2 : 1;
        assert(new_capacity > capacity_);

        T* tmp = static_cast<T*>(::operator new(sizeof(T) * new_capacity, std::align_val_t{alignof(T)}));

        if constexpr (std::is_trivially_copyable_v<T>) {
            std::memcpy(tmp, data_, sizeof(T) * size_);
            if constexpr (!std::is_trivially_destructible_v<T>) {
                for (size_t i = 0; i < size_; ++i) {
                    data_[i].~T();
                }
            }
        } else {
            for (size_t i = 0; i < size_; ++i) {
                new (tmp + i) T{std::move_if_noexcept(data_[i])};
                if constexpr (!std::is_trivially_destructible_v<T>) {
                    data_[i].~T();
                }
            }
        }

        ::operator delete(data_, sizeof(T) * capacity_, std::align_val_t{alignof(T)});
        data_ = tmp;
        capacity_ = new_capacity;
    }
};

class Foo {
public:
    Foo()
    {
        std::println("Foo Created!");
    }

    ~Foo()
    {
        std::println("Foo Destroyed!");
    }
};

int main(void)
{
    // Vector<float> v;
    // for (size_t i = 1; i < 50; ++i) {
    //     v.emplace_back(static_cast<float>(i));
    // }
    // v.push_back(50.0f);
    // v.pop_back();
    // v.println();

    Vector<Foo> w{50};
    for (size_t i = 0; i < 100; ++i) {
        w.emplace_back();
    }

    // for (size_t i = 0; i < u.size(); ++i) {
    //     std::println("Foo");
    // }

    // Vector<std::string> ss;
    // const std::string def = "Hello";
    // for (size_t i = 0; i < 30; ++i) {
    //     ss.emplace_back(def);
    // }
    // ss.println();
}

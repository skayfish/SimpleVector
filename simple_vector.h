#pragma once

#include <cassert>
#include <initializer_list>
#include <algorithm>
#include <string>
#include <stdexcept>

#include "array_ptr.h"

class ReserveProxyObj {
public:
    ReserveProxyObj(size_t capacity)
        : capacity_(capacity) {
    }

    size_t GetCapacity() const noexcept {
        return capacity_;
    }
private:
    size_t capacity_;
};

ReserveProxyObj Reserve(size_t capacity_to_reserve) {
    return ReserveProxyObj(capacity_to_reserve);
}

template <typename Type>
class SimpleVector {
public:
    using Iterator = Type*;
    using ConstIterator = const Type*;

    SimpleVector() noexcept = default;

    explicit SimpleVector(size_t size) 
        :items_(size) {

        if (size) {
            std::fill(begin(), begin() + size, Type());
            size_ = size;
            capacity_ = size;
        }
    }

    SimpleVector(size_t size, const Type& value) 
        :items_(size) {

        if (size) {
            std::fill(begin(), begin() + size, Type(value));
            size_ = size;
            capacity_ = size;
        }
    }

    SimpleVector(std::initializer_list<Type> init) 
        :items_(init.size()) {

        std::copy(init.begin(), init.end(), begin());
        size_ = init.size();
        capacity_ = init.size();
    }

    SimpleVector(const SimpleVector& other)
        :items_(other.capacity_) {

        std::copy(other.begin(), other.end(), begin());
        size_ = other.size_;
    }

    SimpleVector(SimpleVector&& other) noexcept {
        swap(other);
    }

    SimpleVector& operator=(const SimpleVector& rhs) {
        if (this != &rhs) {
            SimpleVector<Type> tmp(rhs);
            swap(tmp);
        }

        return *this;
    }

    SimpleVector& operator=(SimpleVector&& rhs) noexcept {
        SimpleVector<Type> tmp(std::move(rhs));
        swap(tmp);

        return *this;
    }

    SimpleVector(const ReserveProxyObj& new_capacity) 
        : items_(new_capacity.GetCapacity())
        , capacity_(new_capacity.GetCapacity()) {
    }

    void Reserve(size_t new_capacity) {
        if (new_capacity > capacity_) {
            ArrayPtr<Type> new_array_ptr(new_capacity);
            std::fill(new_array_ptr.Get(), new_array_ptr.Get() + new_capacity, Type());
            std::copy(begin(), end(), new_array_ptr.Get());
            items_.swap(new_array_ptr);
            capacity_ = new_capacity;
        }
    }

    void PushBack(Type item) {
        if (size_ < capacity_) {
            items_[size_] = std::move(item);
            ++size_;
        } else {
            size_t new_capacity = std::max(size_t(1), capacity_ * 2);
            ArrayPtr<Type> new_array_ptr(new_capacity);
            std::copy(std::make_move_iterator(begin()), std::make_move_iterator(end()), new_array_ptr.Get());
            new_array_ptr[size_] = std::move(item);
            new_array_ptr.swap(items_);
            size_++;
            capacity_ = new_capacity;
        }
    }

    Iterator Insert(ConstIterator pos, Type value) {
        Iterator result;
        size_t index = pos - begin();
        if (size_ < capacity_) {
            std::copy_backward(std::make_move_iterator(&items_[index]), std::make_move_iterator(end()), end() + 1);
            items_[index] = std::move(value);
            result = &items_[index];
            ++size_;
        } else {
            size_t new_capacity = std::max(size_t(1), capacity_ * 2);

            ArrayPtr<Type> new_array_ptr(new_capacity);
            std::copy(std::make_move_iterator(begin()), std::make_move_iterator(&items_[index]), new_array_ptr.Get());
            new_array_ptr[index] = std::move(value);
            std::copy(std::make_move_iterator(&items_[index]), std::make_move_iterator(end()), new_array_ptr.Get() + index + 1);

            new_array_ptr.swap(items_);
            result = &items_[index];
            size_++;
            capacity_ = new_capacity;
        }

        return result;
    }

    void PopBack() noexcept {
        --size_;
    }

    Iterator Erase(ConstIterator pos) {
        Iterator result;

        size_t index = pos - begin();
        std::copy(std::make_move_iterator(&items_[index + 1]), std::make_move_iterator(end()), &items_[index]);
        result = &items_[index];
        --size_;

        return result;
    }

    void swap(SimpleVector& other) noexcept {
        items_.swap(other.items_);
        std::swap(size_, other.size_);
        std::swap(capacity_, other.capacity_);
    }

    size_t GetSize() const noexcept {
        return size_;
    }

    size_t GetCapacity() const noexcept {
        return capacity_;
    }

    bool IsEmpty() const noexcept {
        return size_ == 0;
    }

    Type& operator[](size_t index) noexcept {
        return items_[index];
    }

    const Type& operator[](size_t index) const noexcept {
        return items_[index];
    }

    Type& At(size_t index) {
        if (index >= size_) {
            using namespace std::literals;
            throw std::out_of_range("index >= size"s);
        }

        return items_[index];
    }

    const Type& At(size_t index) const {
        if (index >= size_) {
            using namespace std::literals;
            throw std::out_of_range("index >= size"s);
        }

        return items_[index];
    }

    void Clear() noexcept {
        size_ = 0;
    }

    void Resize(size_t new_size) {
        if (new_size <= size_) {
            size_ = new_size;
        } else if (new_size <= capacity_) {
            for (size_t i = size_; i < new_size; i++) {
                items_[i] = Type();
            }
        } else {
            size_t new_capacity = std::max(capacity_ * 2, new_size);
            ArrayPtr<Type> new_array(new_capacity);

            std::fill(new_array.Get(), new_array.Get() + new_capacity, Type());
            std::copy(begin(), end(), new_array.Get());

            new_array.swap(items_);
            size_ = new_size;
            capacity_ = new_capacity;
        }
    }

    Iterator begin() noexcept {
        return Iterator(items_.Get());
    }

    Iterator end() noexcept {
        return Iterator(begin() + size_);
    }

    ConstIterator begin() const noexcept {
        return ConstIterator(items_.Get());
    }

    ConstIterator end() const noexcept {
        return ConstIterator(begin() + size_);
    }

    ConstIterator cbegin() const noexcept {
        return ConstIterator(items_.Get());
    }

    ConstIterator cend() const noexcept {
        return ConstIterator(begin() + size_);
    }
private:
    ArrayPtr<Type> items_{};

    size_t size_ = 0;
    size_t capacity_ = 0;
};

template <typename Type>
inline bool operator==(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    if (lhs.GetSize() == rhs.GetSize()) {
        return std::equal(lhs.begin(), lhs.end(), rhs.begin());
    }
    
    return false;
}

template <typename Type>
inline bool operator!=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs == rhs);
}

template <typename Type>
inline bool operator<(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}

template <typename Type>
inline bool operator<=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(rhs < lhs);
}

template <typename Type>
inline bool operator>(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs <= rhs);
}

template <typename Type>
inline bool operator>=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs < rhs);
}

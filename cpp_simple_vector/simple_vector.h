#pragma once

#include <cassert>
#include <initializer_list>
#include "array_ptr.h"

using namespace std;

//Вспомогательный класс для конструктора reserve
class ReserveProxyObj{
    public:
    ReserveProxyObj(size_t size)
        :capacity_obj_(size){}
    
    size_t capacity_obj_ = 0;   
}; 

ReserveProxyObj Reserve(size_t capacity_to_reserve) {
    return ReserveProxyObj(capacity_to_reserve);
}




template <typename Type>
class SimpleVector{

public:
    using Iterator = Type*;
    using ConstIterator = const Type*;

    SimpleVector() noexcept = default;
    ~SimpleVector() = default;
    
   // Создаёт вектор из size элементов, инициализированных значением по умолчанию
    explicit SimpleVector(size_t size)
        : size_{ size },
        capacity_{ size },
        items_{ size }
    {
        std::fill(begin(), end(), Type{});
    }

    // Создаёт вектор из size элементов, инициализированных значением value
    SimpleVector(size_t size, const Type& value)
        : SimpleVector(size)
    {
        std::fill(begin(), end(), value);
    }
    
    // Конструктор reserve, резервирующий нужное кол-во памяти
    explicit SimpleVector(ReserveProxyObj a)
       : size_(0), capacity_(a.capacity_obj_)
    {
    }
    
    // Создаёт вектор из std::initializer_list
    SimpleVector(std::initializer_list<Type> init)
        : SimpleVector(init.size())
    {
        std::copy(init.begin(), init.end(), begin());
    }
    
    // Конструктор - другой вектор
    SimpleVector(const SimpleVector& other)
        : SimpleVector(other.GetSize())
    {
        std::copy(other.begin(), other.end(), begin());
    }
    
    // Конструктор перемещения
    SimpleVector(SimpleVector&& other)
        : size_{ other.size_ },
        capacity_{ other.capacity_ },
        items_{ std::move(other.items_) }
    {
        other.size_ = 0;
        other.capacity_ = 0;

    }
    
    // Возвращает количество элементов в массиве
    size_t GetSize() const noexcept {
        return size_;
    }
    
    // Возвращает вместимость массива
    size_t GetCapacity() const noexcept {
        return capacity_;
    }
    // Сообщает, пустой ли массив
    bool IsEmpty() const noexcept {
        return size_ == 0u;
    }

    // Возвращает ссылку на элемент с индексом index 
    Type& operator[](size_t index) noexcept {
        assert(index < size_);
        return items_[index];
    }
    
    // Возвращает константную ссылку на элемент с индексом index
    const Type& operator[](size_t index) const noexcept {
        assert(index < size_);
        return items_[index];
    }
    
    // Возвращает константную ссылку на элемент с индексом index
    // Выбрасывает исключение std::out_of_range, если index >= size
    const Type& At(size_t index) const {
        if (index >= size_)
            throw std::out_of_range("");
        return items_[index];
    }
    // Обнуляет размер массива, не изменяя его вместимость
    void Clear() noexcept {
        size_ = 0u;
    }

    // Изменяет размер массива.
    void Resize(size_t new_size){
        if ((new_size > size_) && (new_size <= capacity_)) {
             auto it = end();
             while (it != std::next(end(), new_size + size_)){
             *it = std::move(Type());
             it--;
             }
        }

        else if (new_size > capacity_) {
            new_size = std::max(new_size, 2 * size_);
            Reserve(new_size);
            for (std::size_t i{ size_ }; i < new_size; ++i)
            {
                items_[i] = Type{};
            }
        }
        size_ = new_size;
    }
    
    // Возвращает итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    Iterator begin() noexcept {
        return Iterator{ items_.Get() };
    }
    // Возвращает итератор на элемент, следующий за последним
    Iterator end() noexcept {
        return Iterator{ items_.Get() + size_ };
    }
    // Возвращает константный итератор на начало массива
    ConstIterator begin() const noexcept {
        return ConstIterator{ items_.Get() };
    }
    // Возвращает итератор на элемент, следующий за последним
    ConstIterator end() const noexcept {
        return ConstIterator{ items_.Get() + size_ };
    }
    // Возвращает константный итератор на начало массива
    ConstIterator cbegin() const noexcept {
        return ConstIterator{ items_.Get() };
    }
    // Возвращает итератор на элемент, следующий за последним
    ConstIterator cend() const noexcept {
        return ConstIterator{ items_.Get() + size_ };
    }
    
    //Копирование
    SimpleVector& operator=(const SimpleVector& rhs){
        SimpleVector Temp(rhs.GetSize());
            for (size_t i = 0; i < rhs.GetSize(); i++){
                Temp[i] = rhs.At(i);
            }

            items_.swap(Temp.items_);
            size_ = rhs.size_; capacity_ = rhs.capacity_;
        return *this;
    }
    
    //Перемещение
    SimpleVector& operator=(SimpleVector&& rhs){
        SimpleVector Temp(rhs.GetSize());
        
            for (size_t i = 0; i < rhs.GetSize(); i++){
                Temp[i] = rhs.At(i);
            }

            items_.swap(Temp.items_);
            size_ = rhs.size_; capacity_ = rhs.capacity_;
        return *this;
    }
    
    // Добавляет элемент в конец вектора
    // При нехватке места увеличивает вдвое вместимость вектора
    void PushBack(const Type& item){
        if (size_ < capacity_){
            items_[size_++] = item;
            return;
        }
        Reserve(capacity_ ? capacity_ * 2U : 1U);
        items_[size_++] = item;
    }
    
    //Перемещение элемента в конец вектора
    void PushBack(Type&& item) {
        if (size_ < capacity_){
            items_[size_++] = std::move(item);
            return;
        }
        Reserve(capacity_ ? capacity_ * 2U : 1U);
        items_[size_++] = std::move(item);
    }


    // Вставляет значение value в позицию pos.
    // Возвращает итератор на вставленное значение
    // Если перед вставкой значения вектор был заполнен полностью,
    // вместимость вектора должна увеличивается вдвое, а для вектора вместимостью 0 становится равной 1
    Iterator Insert(ConstIterator pos, const Type& value) {
        assert(pos >= begin() && pos <= end());
        auto shift{ std::distance(cbegin(), pos) };
        if (size_ == capacity_){
            Reserve(capacity_ ? capacity_ * 2U : 1U);
        }
        auto pos2{
            const_cast<Iterator>(
                std::next(cbegin(), shift)) };

        std::move_backward(
            pos2, end(), std::next(end()));

        *pos2 = value;
        ++size_;

        return pos2;
    }
    
    //Вставляет элемент в произвольное место контейнера.
    Iterator Insert(ConstIterator pos, Type&& value) {
        assert(pos >= begin() && pos <= end());
        auto shift{ std::distance(cbegin(), pos) };
        if (size_ == capacity_){
            Reserve(capacity_ ? capacity_ * 2U : 1U);
        }
        auto pos2{
            const_cast<Iterator>(
                std::next(cbegin(), shift)) };

        std::move_backward(
            pos2, end(), std::next(end()));

        *pos2 = std::move(value);
        ++size_;

        return pos2;
    }
    
    // "Удаляет" последний элемент вектора. 
    void PopBack() noexcept {
        if (!size_) {return;} else
        --size_;
    }
    // Удаляет элемент вектора в указанной позиции
    Iterator Erase(ConstIterator pos) {
        assert(pos >= begin() && pos < end());
        Iterator pos2{ const_cast<Iterator>(pos) };
        std::move(
            std::next(pos2), end(), pos2);
        --size_;
        return pos2;
    }
    
    // Обменивает значение с другим вектором
    void swap(SimpleVector& other) noexcept {
        items_.swap(other.items_);
        std::swap(size_, other.size_);
        std::swap(capacity_, other.capacity_);
    }

    //Резервирование
    void Reserve(size_t new_cap)
    {
        if (new_cap <= capacity_)
            return;

        ArrayPtr<Type> tmp_items(new_cap);
        for (std::size_t i{}; i < size_; ++i)
        {
            tmp_items[i] = std::move(items_[i]);
        }
        items_.swap(tmp_items);
        capacity_ = new_cap;
    }

private:  
    
    size_t size_{}; // Размер
    size_t capacity_{}; // Вместимость
    ArrayPtr<Type> items_{};


}; //Конец класса simple_vector



//Операции для вектора.
template <typename Type>
inline bool operator==(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return (std::equal(lhs.begin(), lhs.end(), rhs.begin(), rhs.end()) && lhs.GetSize() == rhs.GetSize());
}

template <typename Type>
inline bool operator!=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
   return !(std::equal(lhs.begin(), lhs.end(), rhs.begin(), rhs.end()) && lhs.GetSize() == rhs.GetSize());
}

template <typename Type>
inline bool operator<(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return std::lexicographical_compare(lhs.begin(), lhs.end(),
                             rhs.begin(), rhs.end());
}

template <typename Type>
inline bool operator<=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return  !(std::lexicographical_compare(rhs.begin(), rhs.end(),
                             lhs.begin(), lhs.end()));
}

template <typename Type>
inline bool operator>(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return (std::lexicographical_compare(rhs.begin(), rhs.end(),
                             lhs.begin(), lhs.end()));
}

template <typename Type>
inline bool operator>=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(std::lexicographical_compare(lhs.begin(), lhs.end(),
                             rhs.begin(), rhs.end()));
} 




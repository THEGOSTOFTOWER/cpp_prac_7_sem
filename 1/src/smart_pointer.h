#include <iostream>
#include <utility>

template<typename T>
class SmartPointer {
private:
    T* ptr;
    int* ref_count;

    void cleanup() {
        if (ref_count) {
            --(*ref_count);
            if (*ref_count == 0) {
                delete ptr;
                delete ref_count;
            }
        }
    }

public:
    // Конструкторы
    SmartPointer() : ptr(nullptr), ref_count(nullptr) {}
    
    explicit SmartPointer(T* p) : ptr(p), ref_count(new int(1)) {}
    
    // Конструктор копирования
    SmartPointer(const SmartPointer& other) : ptr(other.ptr), ref_count(other.ref_count) {
        if (ref_count) {
            ++(*ref_count);
        }
    }
    
    // Конструктор перемещения
    SmartPointer(SmartPointer&& other) noexcept : ptr(other.ptr), ref_count(other.ref_count) {
        other.ptr = nullptr;
        other.ref_count = nullptr;
    }
    
    // Деструктор
    ~SmartPointer() {
        cleanup();
    }
    
    // Оператор присваивания копированием
    SmartPointer& operator=(const SmartPointer& other) {
        if (this != &other) {
            cleanup();
            ptr = other.ptr;
            ref_count = other.ref_count;
            if (ref_count) {
                ++(*ref_count);
            }
        }
        return *this;
    }
    
    // Оператор присваивания перемещением
    SmartPointer& operator=(SmartPointer&& other) noexcept {
        if (this != &other) {
            cleanup();
            ptr = other.ptr;
            ref_count = other.ref_count;
            other.ptr = nullptr;
            other.ref_count = nullptr;
        }
        return *this;
    }
    
    // Разыменование
    T& operator*() const { return *ptr; }
    T* operator->() const { return ptr; }
    
    // Получение сырого указателя
    T* get() const { return ptr; }
    
    // Сброс
    void reset() {
        cleanup();
        ptr = nullptr;
        ref_count = nullptr;
    }
    
    void reset(T* p) {
        cleanup();
        ptr = p;
        ref_count = new int(1);
    }
    
    // Обмен
    void swap(SmartPointer& other) {
        std::swap(ptr, other.ptr);
        std::swap(ref_count, other.ref_count);
    }
    
    // Получение количества ссылок
    int use_count() const {
        return ref_count ? *ref_count : 0;
    }
    
    // Проверка на валидность
    explicit operator bool() const {
        return ptr != nullptr;
    }
};

// Операторы сравнения
template<typename T>
bool operator==(const SmartPointer<T>& lhs, const SmartPointer<T>& rhs) {
    return lhs.get() == rhs.get();
}

template<typename T>
bool operator!=(const SmartPointer<T>& lhs, const SmartPointer<T>& rhs) {
    return !(lhs == rhs);
}

template<typename T>
bool operator<(const SmartPointer<T>& lhs, const SmartPointer<T>& rhs) {
    return lhs.get() < rhs.get();
}

template<typename T>
bool operator>(const SmartPointer<T>& lhs, const SmartPointer<T>& rhs) {
    return rhs < lhs;
}

template<typename T>
bool operator<=(const SmartPointer<T>& lhs, const SmartPointer<T>& rhs) {
    return !(rhs < lhs);
}

template<typename T>
bool operator>=(const SmartPointer<T>& lhs, const SmartPointer<T>& rhs) {
    return !(lhs < rhs);
}
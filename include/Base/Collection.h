#pragma once

template <typename T>
class Collection {
   public:
    T* add(const String& name, const String& assign);
    T* at(size_t index);
    T* get(const String name);
    size_t count();

   private:
    T* last();

   private:
    std::vector<T*> _list;
};
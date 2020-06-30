#pragma once

#include <Arduino.h>

class Page {
   private:
    String _name;

   public:
    Page(){};

    const String getName() const {
        return _name;
    }
}
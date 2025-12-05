#pragma once

#include "TFunction.h"
#include <vector>

class FuncFactory {
public:
    static TFunctionPtr Create(const std::string& type);
    static TFunctionPtr Create(const std::string& type, double param);
    static TFunctionPtr Create(const std::string& type, const std::vector<double>& params);
};
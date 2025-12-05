#include "FuncFactory.h"
#include "IdentFunction.h"
#include "ConstFunction.h"
#include "PowerFunction.h"
#include "ExpFunction.h"
#include "PolynomialFunction.h"
#include <stdexcept>

TFunctionPtr FuncFactory::Create(const std::string& type) {
    if (type == "ident") {
        return std::make_shared<IdentFunction>();
    } else if (type == "exp") {
        return std::make_shared<ExpFunction>();
    }
    throw std::invalid_argument("Invalid function type");
}

TFunctionPtr FuncFactory::Create(const std::string& type, double param) {
    if (type == "const") {
        return std::make_shared<ConstFunction>(param);
    } else if (type == "power") {
        return std::make_shared<PowerFunction>(param);
    }
    throw std::invalid_argument("Invalid function type or parameters");
}

TFunctionPtr FuncFactory::Create(const std::string& type, const std::vector<double>& params) {
    if (type == "polynomial") {
        return std::make_shared<PolynomialFunction>(params);
    }
    throw std::invalid_argument("Invalid function type or parameters");
}
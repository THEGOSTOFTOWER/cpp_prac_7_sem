#include <iostream>
#include "TFunction.h"
#include "FuncFactory.h"
#include "Operators.h"
#include "GradientDescent.h"

int main() {
    FuncFactory funcFactory;
    std::vector<TFunctionPtr> cont;

    auto f1 = funcFactory.Create("ident"); 
    auto f2 = FuncFactory::Create("const", 2);
    auto f = *f1 + *f2;



    auto pol = FuncFactory::Create("polynomial", {-3, 1});

    auto res = f * *pol;

    std::cout << res.ToString() << std::endl;

    std::cout << FindRootByGradientDescent(res, -20, 0.1, 1000) << std::endl;


    return 0;
}
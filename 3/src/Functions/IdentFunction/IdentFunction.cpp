#include "IdentFunction.h"

IdentFunction::IdentFunction()
    : TFunction(
        [](double x) { return x; },
        [](double x) { return 1.0; },
        "x"
    ) {}
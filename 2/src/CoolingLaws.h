#include "Abstracts/AbstractCooling.h"
#include <cmath>

class CoolingLawBol : public AbstractCoolingLaw {
    public:
    CoolingLawBol(double initialTemperature) : initialTemperature(initialTemperature) {}

    double cool(double temperature, int iteration) {
        return initialTemperature / std::log(1 + iteration + 1);
    }

    private:
    double initialTemperature;
};


class CoolingLawKosh : public AbstractCoolingLaw {
    public:
    CoolingLawKosh(double initialTemperature) : initialTemperature(initialTemperature) {}
    
    double cool(double temperature, int iteration) {
        return initialTemperature / (1 + iteration + 1);
    }

    private:
    double initialTemperature;
};


class CoolingLawSM : public AbstractCoolingLaw {
    public:
    CoolingLawSM(double initialTemperature) : initialTemperature(initialTemperature) {}

    double cool(double temperature, int iteration) {
        return initialTemperature * std::log(1 + iteration + 1) / (1 + iteration + 1);
    }

    private:
    double initialTemperature;
};

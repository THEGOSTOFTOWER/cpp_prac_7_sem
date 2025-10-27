#pragma once

#include <memory>


class AbstractSolution {
public:
    virtual double getCost() const = 0;
    virtual void print() const = 0;
    virtual std::shared_ptr<AbstractSolution> cloneWithNewSeed(unsigned int seed) const = 0;
    virtual std::shared_ptr<AbstractSolution> clone() const = 0;
};
#include "AbstractSolution.h"

class AbstractMutation {
public:
    virtual ~AbstractMutation() = default;
    virtual void mutate(AbstractSolution& solution) = 0;
};
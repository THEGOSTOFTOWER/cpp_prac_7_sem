#include "Abstracts/AbstractMutation.h"
#include "Solution/Solution.h"

class SchedulingMutation : public AbstractMutation {
public:
    void mutate(AbstractSolution &abstractSolution) override {
        Solution &solution = dynamic_cast<Solution &>(abstractSolution);
        std::mt19937 &rng = solution.getRng();
        std::uniform_int_distribution<int> &distribution = solution.getDistribution();
        std::uniform_int_distribution<int> jobDist(0, solution.getNumJobs() - 1);

        int jobIndex = jobDist(rng);
        int oldProcessor = solution.getJobProcessor(jobIndex);
        int newProcessor = distribution(rng);
        while (newProcessor == oldProcessor) {
            newProcessor = distribution(rng);
        }
        // std::cout << "M1\n";
        solution.updateSchedule(jobIndex, oldProcessor, newProcessor);
        // std::cout << "M2\n";
    }
};
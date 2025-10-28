#include "Abstracts/AbstractSolution.h"
#include <vector>
#include <random>
#include <limits>
#include <cmath>
#include <algorithm>
#include <iostream>

class Solution: public AbstractSolution {
public:
    Solution(int numJobs, int numProcessors, const std::vector<uint32_t> &jobDurations, unsigned int seed)
        : numJobs(numJobs), numProcessors(numProcessors), jobDurations(jobDurations), distribution(0, numProcessors - 1) {
        // std::cout << "HERE_S\n";
        rng.seed(seed);
        schedule.assign(numJobs, std::vector<uint32_t>(numProcessors, 0));
        processorLoads.resize(numProcessors, 0);
        for (int i = 0; i < numJobs; ++i) {
            int processor = distribution(rng);
            schedule[i][processor] = 1;
            processorLoads[processor] += jobDurations[i];
        }
        // std::cout << validateSolution() << std::endl;
        
        // std::cout << "HERE_S1\n";

    }

    double getCost() const override {
        int Tmax = *std::max_element(processorLoads.begin(), processorLoads.end());
        // int Tmin = INT32_MAX;
        // for (int i = 0; i < numProcessors; i++) {
        //     int tmp = INT32_MAX;
        //     for (int j = 0; j < numJobs; j++) {
        //         if (schedule[j][i] == 1 and jobDurations[j] < tmp ) {
        //             tmp = jobDurations[j];
        //         }
        //     }
        //     if (tmp < Tmin) {
        //             Tmin = tmp;
        //         }
        // }
        int Tmin = *std::min_element(jobDurations.begin(), jobDurations.end());
        return static_cast<double>(Tmax - Tmin);
    }

    void print() const override {
        for (int i = 0; i < numProcessors; ++i) {
            std::cout << "Processor " << i << ": Load = " << processorLoads[i] << std::endl;
        }
    }

    std::shared_ptr<AbstractSolution> cloneWithNewSeed(unsigned int seed) const override {
        auto cloned = std::make_shared<Solution>(*this);
        cloned->rng.seed(seed); // Устанавливаем новый seed
        return cloned;
    }

    std::shared_ptr<AbstractSolution> clone() const override { 
        return std::make_shared<Solution>(*this);
    }

    void updateSchedule(int jobIndex, int oldProcessor, int newProcessor) {
        while (true) {
            schedule[jobIndex][oldProcessor] = 0;
            schedule[jobIndex][newProcessor] = 1;
            processorLoads[oldProcessor] -= jobDurations[jobIndex];
            processorLoads[newProcessor] += jobDurations[jobIndex];
            if (validateSolution()) {
                break;
            }
        }
    }

    int getNumJobs() const { return numJobs; }

    int getNumProcessors() const { return numProcessors; }
    
    int getJobProcessor(int jobIndex) const {
        for (int j = 0; j < numProcessors; ++j) {
            if (schedule[jobIndex][j] == 1) {
                return j;
            }
        }
        return -1;
    }

    std::mt19937 &getRng() { return rng; }
    std::uniform_int_distribution<int> &getDistribution() { return distribution; }

    std::vector<std::vector<uint32_t>> &getSch() { return schedule; }

    bool validateSolution() {
        int count = 0;
        for (int j = 0; j < numJobs; j ++) {
            int cnt_tmp = 0;
            for (int i = 0; i < numProcessors; i ++) {
                cnt_tmp += schedule[j][i];
            }
            if (cnt_tmp != 1) {
                return false;
            }
            count += cnt_tmp;
        }
        if (count != numJobs) {
                return false;
            }
            return true;
    }


private:
    int numJobs;
    int numProcessors;
    std::vector<uint32_t> jobDurations;
    std::vector<std::vector<uint32_t>> schedule;
    std::vector<int> processorLoads;
    mutable std::mt19937 rng;
    std::uniform_int_distribution<int> distribution;
};
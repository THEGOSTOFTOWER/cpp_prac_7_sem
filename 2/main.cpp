#include <iostream>
#include <vector>
#include <algorithm>
#include <random>
#include <cmath>
#include <limits>
#include <chrono>
#include <memory>
#include <fstream>
#include <sstream>
#include <thread>
#include <mutex>
#include <atomic>
#include "Mutation/Mutation.h"
#include "CoolingLaws.h"




std::shared_ptr<AbstractSolution> globalBestSolution;



class ParallelSimulatedAnnealing {
public:
    ParallelSimulatedAnnealing(AbstractSolution *solution, SchedulingMutation *mutationOperation, CoolingLawBol *coolingSchedule, double initialTemperature, int maxNoImprovementCount, int threadID, unsigned int seed)
        : initialSolution(solution->cloneWithNewSeed(seed)), mutationOperation(mutationOperation), coolingSchedule(coolingSchedule), temperature(initialTemperature),  maxNoImprovementCount(maxNoImprovementCount), threadID(threadID), rng(seed) {}

    void run() {
        // std::cout << "here\n";
        int iteration = 0;
        double bestCost = initialSolution->getCost(); 
        auto bestSolution = initialSolution->clone(); 
        int noImprovementCount = 0;                   

        while (noImprovementCount < maxNoImprovementCount) {
            
            auto currentSolution = bestSolution->clone();
            mutationOperation->mutate(*currentSolution);
            double currentCost = currentSolution->getCost(); 

            if (currentCost < bestCost) {
                
                bestCost = currentCost;
                noImprovementCount = 0;
                bestSolution = currentSolution;
            } else {
                
                double acceptanceProbability = std::exp(-(currentCost - bestCost) / temperature);
                if (acceptanceProbability >= static_cast<double>(rand()) / RAND_MAX) {
                    
                    noImprovementCount = 0;
                    bestSolution = currentSolution;
                } else {
                    
                    noImprovementCount++;
                }
            }
            
            temperature = coolingSchedule->cool(temperature, iteration);
            iteration++;
        }
        
        localBestSolution = bestSolution;
    }

    std::shared_ptr<AbstractSolution> getLocalBestSolution() const {
        return localBestSolution;
    }

private:
    std::shared_ptr<AbstractSolution> initialSolution;
    std::shared_ptr<AbstractSolution> localBestSolution;
    SchedulingMutation *mutationOperation;
    CoolingLawBol *coolingSchedule;
    double temperature;
    int maxIterations;
    int maxNoImprovementCount;
    int threadID;
    std::mt19937 rng;
};

std::vector<uint32_t> loadJobDurationsFromCSV(const std::string &filename) {
    std::vector<uint32_t> jobDurations;
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Unable to open file " + filename);
    }

    std::string line;
    bool isHeader = true;
    while (std::getline(file, line)) {
        if (isHeader) {
            isHeader = false;
            continue;
        }
        std::stringstream ss(line);
        std::string jobId;
        std::string durationStr;

        std::getline(ss, jobId, ',');
        std::getline(ss, durationStr, ',');

        uint32_t duration = std::stoi(durationStr);
        jobDurations.push_back(duration);
    }

    file.close();
    return jobDurations;
}

int main(int argc, char *argv[]) {
    try {
        if (argc != 2) {
            std::cerr << "Usage: " << argv[0] << " <numThreads>" << std::endl;
            return 1;
        }

        int numThreads = std::stoi(argv[1]);
        std::vector<uint32_t> jobDurations = loadJobDurationsFromCSV("jobs.csv");
        int numJobs = jobDurations.size();
        int numProcessors = 40;
        int maxNoImprovementCount = 100;
        int maxGlobalNoImprovementCount = 10;

        SchedulingMutation mutationOperation;
        
        double initialTemperature = 100.0;
        CoolingLawBol coolingSchedule(initialTemperature);
        int globalNoImprovementCount = 0;

        
        if (!globalBestSolution) {
            globalBestSolution = std::make_shared<Solution>(numJobs, numProcessors, jobDurations, std::chrono::system_clock::now().time_since_epoch().count());
        }
        

        while (globalNoImprovementCount < maxGlobalNoImprovementCount) {
            std::vector<std::thread> threads;
            std::vector<std::shared_ptr<AbstractSolution>> localBestSolutions(numThreads);

            for (int i = 0; i < numThreads; ++i) {
                threads.emplace_back([&, i]() {
                    unsigned int seed = std::chrono::system_clock::now().time_since_epoch().count() + i;
                    std::shared_ptr<AbstractSolution> initialSolution;

                    
                    initialSolution = globalBestSolution->cloneWithNewSeed(seed);
                    

                    ParallelSimulatedAnnealing sa(initialSolution.get(), &mutationOperation, &coolingSchedule, initialTemperature, maxNoImprovementCount, i, seed);
                    sa.run();

                    localBestSolutions[i] = sa.getLocalBestSolution();
                });
            }

            for (auto &t : threads) {
                t.join();
            }

            bool improved = false;
            for (const auto &localBest : localBestSolutions) {
                if (localBest->getCost() < globalBestSolution->getCost()) {
                    globalBestSolution = localBest;
                    improved = true;
                }
            }
            if (improved) {
                globalNoImprovementCount = 0;
            } else {
                globalNoImprovementCount++;
            }

            std::cout << "Current best solution cost: " << globalBestSolution->getCost() << std::endl;
        }
        std::cout << "Current best solution cost: " << globalBestSolution->getCost() << std::endl;
    } catch (const std::exception &e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    return 0;
}
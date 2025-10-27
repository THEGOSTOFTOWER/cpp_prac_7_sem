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



class SimulatedAnnealing
{
public:
    SimulatedAnnealing(Solution *solution, SchedulingMutation *mutationOperation, AbstractCoolingLaw *coolingSchedule, double initialTemperature, int maxIterations, int maxNoImprovementCount)
        : solution(solution), mutationOperation(mutationOperation), coolingSchedule(coolingSchedule), temperature(initialTemperature), maxIterations(maxIterations), maxNoImprovementCount(maxNoImprovementCount) {}

    void run()
    {
        
        int iteration = 0;
        double bestCost = solution->getCost(); // Изначальная стоимость решения
        auto bestSolution = solution->clone(); // Копия наилучшего решения
        int noImprovementCount = 0;            // Счетчик количества итераций без улучшения
        // if (bestSolution == nullptr) {
        //     std::cout << "trubl\n";
        // }
        while (iteration < maxIterations && noImprovementCount < maxNoImprovementCount)
        {
            // Клонируем лучшее решение и применяем к нему мутацию
            auto currentSolution = bestSolution->clone();
            // std::cout << "LLV\n";
            // if (currentSolution == nullptr) {
            // std::cout << "trubl\n";
            // }
            mutationOperation->mutate(*currentSolution);
            // std::cout << "LLV1\n";
            double currentCost = currentSolution->getCost(); // Стоимость мутированного решения
            // if (currentSolution == bestSolution) {
            //     std::cout << "WHY\n";
            // }
            // std::cout << "LLV2\n";
            // std ::cout << bestCost << "?" << currentCost << std::endl;
            if (currentCost <= bestCost)
            {
                // Если новое решение лучше, обновляем наилучшее решение
                bestCost = currentCost;
                noImprovementCount = 0;
                bestSolution = currentSolution;
            }
            else
            {
                // Если решение хуже, то принимаем его с некоторой вероятностью (правило Метрополиса)
                // std::cout << "L1\n";
                double acceptanceProbability = std::exp(-(currentCost - bestCost) / temperature);
                double prob = static_cast<double>(rand()) / RAND_MAX;
                //  std::cout << acceptanceProbability << ' ' << prob << std::endl;
                if (acceptanceProbability > prob)
                {
                    // Принять ухудшающее решение и обновить лучшее решение
                    noImprovementCount = 0;
                    bestSolution = currentSolution;
                }
                else
                {
                    // Если решение не принято, увеличиваем счетчик итераций без улучшений
                    noImprovementCount++;
                }
                // std::cout << "L3\n";
            }
            // Обновляем температуру согласно закону понижения температуры
            // std::cout << "L1\n";
            temperature = coolingSchedule->cool(temperature, iteration);
            // std::cout << "L2\n";
            iteration++;
        }
        // Печатаем наилучшее найденное решение
        bestSolution->print();
        std::cout << "Best solution found with cost: " << bestCost << std::endl;
    }

private:
    Solution *solution;                   // Текущее решение
    SchedulingMutation *mutationOperation; // Операция мутации решения
    AbstractCoolingLaw *coolingSchedule;     // План понижения температуры
    double temperature;                   // Текущая температура
    int maxIterations;                    // Максимальное количество итераций
    int maxNoImprovementCount;            // Условие останова или максимально число иттераций без улучшений
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

int main(int argc, char *argv[])
{

    if (argc != 4)
    {
        std::cerr << "Usage: " << argv[0] << " <filename> <num_processors> <cooling_method>" << std::endl;
        std::cerr << "Cooling methods: boltzmann, cauchy, logarithmic" << std::endl;
        return 1;
    }
    std::string filename = argv[1];
    int numProcessors = std::stoi(argv[2]);
    std::string coolingMethod = argv[3];

    // Указатель на план понижения температуры
    std::unique_ptr<AbstractCoolingLaw> coolingSchedule;

    // Инициализация метода понижения температуры в зависимости от параметра
    double initialTemperature = 100.0;
    if (coolingMethod == "boltzmann")
    {
        coolingSchedule = std::make_unique<CoolingLawBol>(initialTemperature);
    }
    else if (coolingMethod == "kosh")
    {
        coolingSchedule = std::make_unique<CoolingLawKosh>(initialTemperature);
    }
    else if (coolingMethod == "SM")
    {
        coolingSchedule = std::make_unique<CoolingLawSM>(initialTemperature);
    }
    else
    {
        std::cerr << "Invalid cooling method. Available methods: boltzmann, cauchy, logarithmic" << std::endl;
        return 1;
    }

    try
    {
        // Загружаем длительности работ из файла
        std::vector<uint32_t> jobDurations = loadJobDurationsFromCSV(filename);
        

        int numJobs = jobDurations.size();

        Solution solution(numJobs, numProcessors, jobDurations, 42);
        SchedulingMutation mutationOperation;

        int maxIterations = 100000;
        int maxNoImprovementCount = 100;
        SimulatedAnnealing sa(&solution, &mutationOperation, coolingSchedule.get(), initialTemperature, maxIterations, maxNoImprovementCount);

        sa.run();
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
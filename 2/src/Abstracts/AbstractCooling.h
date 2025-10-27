class AbstractCoolingLaw {
public:
    virtual ~AbstractCoolingLaw() = default;
    virtual double cool(double temperature, int iteration) = 0;
};
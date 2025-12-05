#include <gtest/gtest.h>
#include "TFunction.h"
#include "FuncFactory.h"
#include "Operators.h"
#include "FuncFactory.h"
#include "IdentFunction.h"
#include "ConstFunction.h"
#include "PowerFunction.h"
#include "ExpFunction.h"
#include "PolynomialFunction.h"
#include "GradientDescent.h"
#include <cmath>

// Тесты для базовых функций
TEST(FunctionTest, IdentityFunction) {
    IdentFunction f;
    EXPECT_DOUBLE_EQ(f(5.0), 5.0);
    EXPECT_DOUBLE_EQ(f.GetDeriv(5.0), 1.0);
    EXPECT_EQ(f.ToString(), "x");
}

TEST(FunctionTest, ConstantFunction) {
    ConstFunction f(3.5);
    EXPECT_DOUBLE_EQ(f(10.0), 3.5);
    EXPECT_DOUBLE_EQ(f.GetDeriv(10.0), 0.0);
    EXPECT_EQ(f.ToString(), "3.5");
}

TEST(FunctionTest, PowerFunction) {
    PowerFunction f(3); // x^3
    EXPECT_DOUBLE_EQ(f(2.0), 8.0);
    EXPECT_DOUBLE_EQ(f.GetDeriv(2.0), 12.0); // 3*x^2 = 3*4 = 12
    EXPECT_EQ(f.ToString(), "x^3");
    
    PowerFunction f1(1); // x
    EXPECT_EQ(f1.ToString(), "x");
    
    PowerFunction f0(0); // 1
    EXPECT_EQ(f0.ToString(), "1");
}

TEST(FunctionTest, ExponentialFunction) {
    ExpFunction f;
    EXPECT_DOUBLE_EQ(f(0.0), 1.0);
    EXPECT_DOUBLE_EQ(f(1.0), std::exp(1.0));
    EXPECT_DOUBLE_EQ(f.GetDeriv(1.0), std::exp(1.0));
    EXPECT_EQ(f.ToString(), "exp(x)");
}

TEST(FunctionTest, PolynomialFunction) {
    PolynomialFunction f({2.0, 3.0, 1.0}); // 2 + 3x + x^2
    EXPECT_DOUBLE_EQ(f(2.0), 2.0 + 3.0*2.0 + 4.0); // 2 + 6 + 4 = 12
    EXPECT_DOUBLE_EQ(f.GetDeriv(2.0), 3.0 + 2.0*2.0); // 3 + 4 = 7
    EXPECT_EQ(f.ToString(), "2 + 3*x + x^2");
}

// Тесты для фабрики
TEST(FuncFactoryTest, CreateFunctions) {
    auto ident = FuncFactory::Create("ident");
    EXPECT_DOUBLE_EQ((*ident)(5.0), 5.0);
    
    auto constant = FuncFactory::Create("const", {3.5});
    EXPECT_DOUBLE_EQ((*constant)(10.0), 3.5);
    
    auto power = FuncFactory::Create("power", {2});
    EXPECT_DOUBLE_EQ((*power)(3.0), 9.0);
    
    auto exp_func = FuncFactory::Create("exp");
    EXPECT_DOUBLE_EQ((*exp_func)(0.0), 1.0);
    
    auto poly = FuncFactory::Create("polynomial", {1.0, 2.0, 3.0});
    EXPECT_DOUBLE_EQ((*poly)(2.0), 1.0 + 4.0 + 12.0);
}

TEST(FuncFactoryTest, InvalidTypeThrows) {
    EXPECT_THROW(FuncFactory::Create("unknown"), std::invalid_argument);
}

// Тесты для арифметических операций
TEST(ArithmeticTest, Addition) {
    ConstFunction c(3.0);
    IdentFunction x;
    
    auto sum = c + x; // 3 + x
    EXPECT_DOUBLE_EQ(sum(2.0), 5.0);
    EXPECT_DOUBLE_EQ(sum.GetDeriv(2.0), 1.0);
}

TEST(ArithmeticTest, Subtraction) {
    PowerFunction x2(2); // x^2
    ConstFunction c(5.0);
    
    auto diff = x2 - c; // x^2 - 5
    EXPECT_DOUBLE_EQ(diff(3.0), 4.0); // 9 - 5 = 4
    EXPECT_DOUBLE_EQ(diff.GetDeriv(3.0), 6.0); // 2x = 6
}

TEST(ArithmeticTest, Multiplication) {
    IdentFunction x;
    ConstFunction c(2.0);
    
    auto product = x * c; // 2x
    EXPECT_DOUBLE_EQ(product(3.0), 6.0);
    EXPECT_DOUBLE_EQ(product.GetDeriv(3.0), 2.0);
}

TEST(ArithmeticTest, Division) {
    PolynomialFunction num({0.0, 2.0}); // 2x
    ConstFunction den(2.0); // 2
    
    auto quotient = num / den; // x
    EXPECT_DOUBLE_EQ(quotient(3.0), 3.0);
    EXPECT_DOUBLE_EQ(quotient.GetDeriv(3.0), 1.0);
}

TEST(ArithmeticTest, ComplexExpression) {
    // f(x) = (x^2 + 3x - 2) * (x + 1)
    auto x_squared = FuncFactory::Create("power", {2});
    auto x_func = FuncFactory::Create("ident");
    auto c3 = FuncFactory::Create("const", {3.0});
    auto c2 = FuncFactory::Create("const", {2.0});
    auto c1 = FuncFactory::Create("const", {1.0});
    
    // x^2 + 3x
    auto term1 = *x_squared + (*c3 * *x_func);
    // (x^2 + 3x) - 2
    auto left_part = term1 - *c2;
    // (x + 1)
    auto right_part = *x_func + *c1;
    
    // (x^2 + 3x - 2) * (x + 1)
    auto complex_func = left_part * right_part;
    
    // Проверим в точке x = 1
    // (1 + 3 - 2) * (1 + 1) = 2 * 2 = 4
    EXPECT_NEAR(complex_func(1.0), 4.0, 1e-10);
}

// Тесты для производных
TEST(DerivativeTest, BasicDerivatives) {
    PowerFunction x3(3); // x^3
    EXPECT_DOUBLE_EQ(x3.GetDeriv(2.0), 12.0); // 3*2^2 = 12
    
    ExpFunction exp;
    EXPECT_DOUBLE_EQ(exp.GetDeriv(0.0), 1.0);
    
    PolynomialFunction poly({1.0, 2.0, 3.0}); // 1 + 2x + 3x^2
    EXPECT_DOUBLE_EQ(poly.GetDeriv(1.0), 2.0 + 6.0); // 2 + 6 = 8
}

TEST(DerivativeTest, ArithmeticDerivatives) {
    // f(x) = x^2 * exp(x)
    auto x2 = FuncFactory::Create("power", {2});
    auto exp_func = FuncFactory::Create("exp");
    
    auto product = *x2 * *exp_func;
    
    double x = 1.0;
    double expected = 2.0*std::exp(1.0) + std::exp(1.0); // 2x*e^x + x^2*e^x = 2e + e = 3e
    EXPECT_NEAR(product.GetDeriv(x), expected, 1e-10);
}

// Тесты для градиентного спуска
TEST(GradientDescentTest, LinearEquation) {
    // f(x) = 2x - 4, корень: x = 2
    auto linear = FuncFactory::Create("polynomial", {-4.0, 2.0});
    
    double root = FindRootByGradientDescent(*linear, 0.0, 0.1, 1000);
    EXPECT_NEAR(root, 2.0, 1e-5);
}

TEST(GradientDescentTest, QuadraticEquation) {
    // f(x) = x^2 - 4, корни: x = ±2
    auto quadratic = FuncFactory::Create("polynomial", {-4.0, 0.0, 1.0});
    
    double root_positive = FindRootByGradientDescent(*quadratic, 5.0, 0.1, 1000);
    EXPECT_NEAR(root_positive, 2.0, 1e-5);
    
    double root_negative = FindRootByGradientDescent(*quadratic, -5.0, 0.1, 1000);
    EXPECT_NEAR(root_negative, -2.0, 1e-5);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
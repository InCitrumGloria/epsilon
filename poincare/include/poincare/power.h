#ifndef POINCARE_POWER_H
#define POINCARE_POWER_H

#include <poincare/static_hierarchy.h>
#include <poincare/approximation_engine.h>
#include <poincare/rational.h>
#include <poincare/multiplication.h>

namespace Poincare {

class Power : public StaticHierarchy<2> {
  using StaticHierarchy<2>::StaticHierarchy;
  friend class Multiplication;
  friend class NthRoot;
  friend class SquareRoot;
  friend class Addition;
  friend class Division;
  friend class Round;
  friend class Symbol;
public:
  Type type() const override;
  Expression * clone() const override;
  Sign sign() const override;
  int polynomialDegree(char symbolName) const override;
  int privateGetPolynomialCoefficients(char symbolName, Expression * coefficients[]) const override;
  template<typename T> static std::complex<T> compute(const std::complex<T> c, const std::complex<T> d);
private:
  constexpr static int k_maxNumberOfTermsInExpandedMultinome = 25;
  constexpr static int k_maxExactPowerMatrix = 100;
  /* Property */
  Expression * setSign(Sign s, Context & context, AngleUnit angleUnit) override;
  /* Layout */
  ExpressionLayout * createLayout(PrintFloat::Mode floatDisplayMode, int numberOfSignificantDigits) const override;
  bool needParenthesisWithParent(const Expression * e) const override;
  int writeTextInBuffer(char * buffer, int bufferSize, PrintFloat::Mode floatDisplayMode, int numberOfSignificantDigits) const override {
    return LayoutEngine::writeInfixExpressionTextInBuffer(this, buffer, bufferSize, floatDisplayMode, numberOfSignificantDigits, name());
  }
  static const char * name() { return "^"; }
  /* Simplify */
  Expression * shallowReduce(Context& context, AngleUnit angleUnit) override;
  Expression * shallowBeautify(Context & context, AngleUnit angleUnit) override;
  int simplificationOrderGreaterType(const Expression * e, bool canBeInterrupted) const override;
  int simplificationOrderSameType(const Expression * e, bool canBeInterrupted) const override;
  Expression * simplifyPowerPower(Power * p, Expression * r, Context & context, AngleUnit angleUnit);
  Expression * cloneDenominator(Context & context, AngleUnit angleUnit) const override;
  Expression * simplifyPowerMultiplication(Multiplication * m, Expression * r, Context & context, AngleUnit angleUnit);
  Expression * simplifyRationalRationalPower(Expression * result, Rational * a, Rational * b, Context & context, AngleUnit angleUnit);
  Expression * removeSquareRootsFromDenominator(Context & context, AngleUnit angleUnit);
  bool parentIsALogarithmOfSameBase() const;
  bool isNthRootOfUnity() const;
  static Expression * CreateSimplifiedIntegerRationalPower(Integer i, Rational * r, bool isDenominator, Context & context, AngleUnit angleUnit);
  static Expression * CreateNthRootOfUnity(const Rational r);
  static bool TermIsARationalSquareRootOrRational(const Expression * e);
  static const Rational * RadicandInExpression(const Expression * e);
  static const Rational * RationalFactorInExpression(const Expression * e);
  static bool RationalExponentShouldNotBeReduced(const Rational * b, const Rational * r);
  /* Evaluation */
  constexpr static int k_maxApproximatePowerMatrix = 1000;
  template<typename T> static MatrixComplex<T> computeOnComplexAndMatrix(const std::complex<T> c, const MatrixComplex<T> n);
  template<typename T> static MatrixComplex<T> computeOnMatrixAndComplex(const MatrixComplex<T> m, const std::complex<T> d);
  template<typename T> static MatrixComplex<T> computeOnMatrices(const MatrixComplex<T> m, const MatrixComplex<T> n);
  Evaluation<float> * privateApproximate(SinglePrecision p, Context& context, AngleUnit angleUnit) const override {
    return ApproximationEngine::mapReduce<float>(this, context, angleUnit, compute<float>, computeOnComplexAndMatrix<float>, computeOnMatrixAndComplex<float>, computeOnMatrices<float>);
  }
  Evaluation<double> * privateApproximate(DoublePrecision p, Context& context, AngleUnit angleUnit) const override {
    return ApproximationEngine::mapReduce<double>(this, context, angleUnit, compute<double>, computeOnComplexAndMatrix<double>, computeOnMatrixAndComplex<double>, computeOnMatrices<double>);
  }
};

}

#endif

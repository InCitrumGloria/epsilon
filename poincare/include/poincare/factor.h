#ifndef POINCARE_FACTOR_H
#define POINCARE_FACTOR_H

#include <poincare/layout_engine.h>
#include <poincare/static_hierarchy.h>
#include <poincare/multiplication.h>
#include <poincare/rational.h>
#include <cmath>

namespace Poincare {

class Factor : public StaticHierarchy<1> {
  using StaticHierarchy<1>::StaticHierarchy;
public:
  Type type() const override;
  Expression * clone() const override;
private:
  /* Layout */
  ExpressionLayout * createLayout(PrintFloat::Mode floatDisplayMode, int numberOfSignificantDigits) const override {
    return LayoutEngine::createPrefixLayout(this, floatDisplayMode, numberOfSignificantDigits, name());
  }
  int writeTextInBuffer(char * buffer, int bufferSize, PrintFloat::Mode floatDisplayMode, int numberOfSignificantDigits) const override {
    return LayoutEngine::writePrefixExpressionTextInBuffer(this, buffer, bufferSize, floatDisplayMode, numberOfSignificantDigits, name());
  }
  const char * name() const { return "factor"; }
  /* Simplification */
  Expression * shallowBeautify(Context& context, AngleUnit angleUnit) override;
  Expression * createMultiplicationOfIntegerPrimeDecomposition(Integer i, Context & context, AngleUnit angleUnit);
  /* Evaluation */
  Evaluation<float> * privateApproximate(SinglePrecision p, Context& context, AngleUnit angleUnit) const override { return templatedApproximate<float>(context, angleUnit); }
  Evaluation<double> * privateApproximate(DoublePrecision p, Context& context, AngleUnit angleUnit) const override { return templatedApproximate<double>(context, angleUnit); }
  template<typename T> Evaluation<T> * templatedApproximate(Context& context, AngleUnit angleUnit) const {
    return operand(0)->privateApproximate(T(), context, angleUnit);
  }
};

}

#endif


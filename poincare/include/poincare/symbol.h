#ifndef POINCARE_SYMBOL_H
#define POINCARE_SYMBOL_H

#include <poincare/static_hierarchy.h>

namespace Poincare {

class Symbol : public StaticHierarchy<0> {
  friend class Store;
public:
  enum SpecialSymbols : char {
    /* We can use characters from 1 to 31 as they do not correspond to usual
     * characters but events as 'end of text', 'backspace'... */
    Ans = 1,
    un = 2,
    un1 = 3,
    un2 = 4,
    vn = 5,
    vn1 = 6,
    vn2 = 7,
    M0 = 8,
    M1 = 9,
    M2,
    M3,
    M4,
    M5,
    M6,
    M7,
    M8,
    M9 = 17,
    V1,
    N1,
    V2,
    N2,
    V3,
    N3 = 23,
    X1 = 24,
    Y1,
    X2,
    Y2,
    X3,
    Y3 = 29
  };
  static SpecialSymbols matrixSymbol(char index);
  Symbol(char name);
  Symbol(Symbol&& other); // C++11 move constructor
  Symbol(const Symbol& other); // C++11 copy constructor
  char name() const;
  Type type() const override;
  Expression * clone() const override;
  int polynomialDegree(char symbolName) const override;
  int privateGetPolynomialCoefficients(char symbolName, Expression * coefficients[]) const override;
  Sign sign() const override;
  bool isMatrixSymbol() const;
  bool isScalarSymbol() const;
  static bool isVariableSymbol(char c);
  static bool isSeriesSymbol(char c);
  static bool isRegressionSymbol(char c);
  bool isApproximate(Context & context) const;
  float characteristicXRange(Context & context, AngleUnit angleUnit) const override;
  bool hasAnExactRepresentation(Context & context) const;
  static const char * textForSpecialSymbols(char name);
  int getVariables(isVariableTest isVariable, char * variables) const override;
private:
  Expression * replaceSymbolWithExpression(char symbol, Expression * expression) override;
  /* Simplification */
  Expression * shallowReduce(Context& context, AngleUnit angleUnit) override;
  /* Comparison */
  int simplificationOrderSameType(const Expression * e, bool canBeInterrupted) const override;
  /* Layout */
  ExpressionLayout * createLayout(PrintFloat::Mode floatDisplayMode, int numberOfSignificantDigits) const override;
  int writeTextInBuffer(char * buffer, int bufferSize, PrintFloat::Mode floatDisplayMode, int numberOfSignificantDigits) const override;
  /* Evaluation */
  Evaluation<float> * privateApproximate(SinglePrecision p, Context& context, AngleUnit angleUnit) const override { return templatedApproximate<float>(context, angleUnit); }
  Evaluation<double> * privateApproximate(DoublePrecision p, Context& context, AngleUnit angleUnit) const override { return templatedApproximate<double>(context, angleUnit); }
 template<typename T> Evaluation<T> * templatedApproximate(Context& context, AngleUnit angleUnit) const;
  const char m_name;
};

}

#endif

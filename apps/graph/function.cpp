#include "function.h"

namespace Graph {

Function::Function(const char * text, KDColor color) :
  Shared::Function(text, color),
  m_displayDerivative(false)
{
}

bool Function::displayDerivative() {
  return m_displayDerivative;
}

void Function::setDisplayDerivative(bool display) {
  m_displayDerivative = display;
}

float Function::approximateDerivative(float x, Poincare::Context * context, Poincare::Expression::AngleUnit angleUnit) const {
  Poincare::Complex abscissa = Poincare::Complex(x);
  Poincare::Expression * args[2] = {m_expression, &abscissa};
  Poincare::Derivative derivative = Poincare::Derivative();
  derivative.setArgument(args, 2, true);
  return derivative.approximate(*context, angleUnit);
}

char Function::symbol() const {
  return 'x';
}

}

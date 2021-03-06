#include <poincare/trigonometry.h>
#include <poincare/hyperbolic_cosine.h>
#include <poincare/symbol.h>
#include <poincare/preferences.h>
#include <poincare/undefined.h>
#include <poincare/rational.h>
#include <poincare/multiplication.h>
#include <poincare/subtraction.h>
#include <poincare/derivative.h>
#include <poincare/decimal.h>
#include <ion.h>
extern "C" {
#include <assert.h>
}
#include <cmath>
#include <float.h>

namespace Poincare {

float Trigonometry::characteristicXRange(const Expression * e, Context & context, Expression::AngleUnit angleUnit) {
  assert(e->numberOfOperands() == 1);
  const Expression * op = e->operand(0);
  int d = op->polynomialDegree('x');
  // op is not linear so we cannot not easily find an interesting range
  if (d < 0 || d > 1) {
    return op->characteristicXRange(context, angleUnit);
  }
  // The expression e is x-independent
  if (d == 0) {
    return 0.0f;
  }
  // e has the form cos/sin/tan(ax+b) so it is periodic of period 2*Pi/a
  assert(d == 1);
  /* To compute a, the slope of the expression op, we compute the derivative of
   * op for any x value. */
  Poincare::Approximation<float> x(1.0f);
  const Poincare::Expression * args[2] = {op, &x};
  Poincare::Derivative derivative(args, true);
  float a = derivative.approximateToScalar<float>(context, angleUnit);
  float pi = angleUnit == Expression::AngleUnit::Radian ? M_PI : 180.0f;
  return 2.0f*pi/std::fabs(a);
}

Expression * Trigonometry::shallowReduceDirectFunction(Expression * e, Context& context, Expression::AngleUnit angleUnit) {
  assert(e->type() == Expression::Type::Sine || e->type() == Expression::Type::Cosine || e->type() == Expression::Type::Tangent);
  Expression * lookup = Trigonometry::table(e->operand(0), e->type(), context, angleUnit);
  if (lookup != nullptr) {
    return e->replaceWith(lookup, true);
  }
  Expression::Type correspondingType = e->type() == Expression::Type::Cosine ? Expression::Type::ArcCosine : (e->type() == Expression::Type::Sine ? Expression::Type::ArcSine : Expression::Type::ArcTangent);
  if (e->operand(0)->type() == correspondingType) {
    return e->replaceWith(e->editableOperand(0)->editableOperand(0), true);
  }
  if (e->operand(0)->sign() == Expression::Sign::Negative) {
    Expression * op = e->editableOperand(0);
    Expression * newOp = op->setSign(Expression::Sign::Positive, context, angleUnit);
    newOp->shallowReduce(context, angleUnit);
    if (e->type() == Expression::Type::Cosine) {
      return e->shallowReduce(context, angleUnit);
    } else {
      Multiplication * m = new Multiplication(new Rational(-1), e->clone(), false);
      m->editableOperand(1)->shallowReduce(context, angleUnit);
      return e->replaceWith(m, true)->shallowReduce(context, angleUnit);
    }
  }
  if ((angleUnit == Expression::AngleUnit::Radian && e->operand(0)->type() == Expression::Type::Multiplication && e->operand(0)->numberOfOperands() == 2 && e->operand(0)->operand(1)->type() == Expression::Type::Symbol && static_cast<const Symbol *>(e->operand(0)->operand(1))->name() == Ion::Charset::SmallPi && e->operand(0)->operand(0)->type() == Expression::Type::Rational) || (angleUnit == Expression::AngleUnit::Degree && e->operand(0)->type() == Expression::Type::Rational)) {
    Rational * r = angleUnit == Expression::AngleUnit::Radian ? static_cast<Rational *>(e->editableOperand(0)->editableOperand(0)) : static_cast<Rational *>(e->editableOperand(0));
    int unaryCoefficient = 1; // store 1 or -1
    // Replace argument in [0, Pi/2[ or [0, 90[
    Integer divisor = angleUnit == Expression::AngleUnit::Radian ? r->denominator() : Integer::Multiplication(r->denominator(), Integer(90));
    Integer dividand = angleUnit == Expression::AngleUnit::Radian ? Integer::Addition(r->numerator(), r->numerator()) : r->numerator();
    if (divisor.isLowerThan(dividand)) {
      Integer piDivisor = angleUnit == Expression::AngleUnit::Radian ? r->denominator() : Integer::Multiplication(r->denominator(), Integer(180));
      IntegerDivision div = Integer::Division(r->numerator(), piDivisor);
      dividand = angleUnit == Expression::AngleUnit::Radian ? Integer::Addition(div.remainder, div.remainder) : div.remainder;
      if (divisor.isLowerThan(dividand)) {
        div.remainder = Integer::Subtraction(piDivisor, div.remainder);
        if (e->type() == Expression::Type::Cosine || e->type() == Expression::Type::Tangent) {
          unaryCoefficient *= -1;
        }
      }
      Rational * newR = new Rational(div.remainder, r->denominator());
      Expression * rationalParent = angleUnit == Expression::AngleUnit::Radian ? e->editableOperand(0) : e;
      rationalParent->replaceOperand(r, newR, true);
      e->editableOperand(0)->shallowReduce(context, angleUnit);
      if (Integer::Division(div.quotient, Integer(2)).remainder.isOne() && e->type() != Expression::Type::Tangent) {
        unaryCoefficient *= -1;
      }
      Expression * simplifiedCosine = e->shallowReduce(context, angleUnit); // recursive
      Multiplication * m = new Multiplication(new Rational(unaryCoefficient), simplifiedCosine->clone(), false);
      return simplifiedCosine->replaceWith(m, true)->shallowReduce(context, angleUnit);
    }
    assert(r->sign() == Expression::Sign::Positive);
    assert(!divisor.isLowerThan(dividand));
  }
  return e;
}

bool Trigonometry::ExpressionIsEquivalentToTangent(const Expression * e) {
  assert(Expression::Type::Power < Expression::Type::Sine);
  if (e->type() == Expression::Type::Multiplication && e->operand(1)->type() == Expression::Type::Sine && e->operand(0)->type() == Expression::Type::Power && e->operand(0)->operand(0)->type() == Expression::Type::Cosine && e->operand(0)->operand(1)->type() == Expression::Type::Rational && static_cast<const Rational *>(e->operand(0)->operand(1))->isMinusOne()) {
    return true;
  }
  return false;
}

Expression * Trigonometry::shallowReduceInverseFunction(Expression * e, Context& context, Expression::AngleUnit angleUnit) {
  assert(e->type() == Expression::Type::ArcCosine || e->type() == Expression::Type::ArcSine || e->type() == Expression::Type::ArcTangent);
  Expression::Type correspondingType = e->type() == Expression::Type::ArcCosine ? Expression::Type::Cosine : (e->type() == Expression::Type::ArcSine ? Expression::Type::Sine : Expression::Type::Tangent);
  float pi = angleUnit == Expression::AngleUnit::Radian ? M_PI : 180;
  if (e->operand(0)->type() == correspondingType) {
    float trigoOp = e->operand(0)->operand(0)->approximateToScalar<float>(context, angleUnit);
    if ((e->type() == Expression::Type::ArcCosine && trigoOp >= 0.0f && trigoOp <= pi) ||
        (e->type() == Expression::Type::ArcSine && trigoOp >= -pi/2.0f && trigoOp <= pi/2.0f) ||
        (e->type() == Expression::Type::ArcTangent && trigoOp >= -pi/2.0f && trigoOp <= pi/2.0f)) {
      return e->replaceWith(e->editableOperand(0)->editableOperand(0), true);
    }
  }
  // Special case for arctan(sin(x)/cos(x))
  if (e->type() == Expression::Type::ArcTangent && ExpressionIsEquivalentToTangent(e->operand(0))) {
    float trigoOp = e->operand(0)->operand(1)->operand(0)->approximateToScalar<float>(context, angleUnit);
    if (trigoOp >= -pi/2.0f && trigoOp <= pi/2.0f) {
      return e->replaceWith(e->editableOperand(0)->editableOperand(1)->editableOperand(0), true);
    }
  }
  Expression * lookup = Trigonometry::table(e->operand(0), e->type(), context, angleUnit);
  if (lookup != nullptr) {
    return e->replaceWith(lookup, true);
  }
  // arccos(-x) = Pi-arcos(x), arcsin(-x) = -arcsin(x), arctan(-x)=-arctan(x)
  if (e->operand(0)->sign() == Expression::Sign::Negative || (e->operand(0)->type() == Expression::Type::Multiplication && e->operand(0)->operand(0)->type() == Expression::Type::Rational && static_cast<const Rational *>(e->operand(0)->operand(0))->isMinusOne())) {
    Expression * op = e->editableOperand(0);
    if (e->operand(0)->sign() == Expression::Sign::Negative) {
      Expression * newOp = op->setSign(Expression::Sign::Positive, context, angleUnit);
      newOp->shallowReduce(context, angleUnit);
    } else {
      ((Multiplication *)op)->removeOperand(op->editableOperand(0), true);
      op->shallowReduce(context, angleUnit);
    }
    if (e->type() == Expression::Type::ArcCosine) {
      Expression * pi = angleUnit == Expression::AngleUnit::Radian ? static_cast<Expression *>(new Symbol(Ion::Charset::SmallPi)) : static_cast<Expression *>(new Rational(180));
      Subtraction * s = new Subtraction(pi, e->clone(), false);
      s->editableOperand(1)->shallowReduce(context, angleUnit);
      return e->replaceWith(s, true)->shallowReduce(context, angleUnit);
    } else {
      Multiplication * m = new Multiplication(new Rational(-1), e->clone(), false);
      m->editableOperand(1)->shallowReduce(context, angleUnit);
      return e->replaceWith(m, true)->shallowReduce(context, angleUnit);
    }
  }

  return e;
}

static_assert('\x8A' == Ion::Charset::SmallPi, "Unicode error");
constexpr const char * cheatTable[Trigonometry::k_numberOfEntries][5] =
{{"-90",    "\x8A*(-2)^(-1)",    "",                                   "-1",                                 "undef"},
 {"-75",    "\x8A*(-5)*12^(-1)", "",                                   "(-1)*6^(1/2)*4^(-1)-2^(1/2)*4^(-1)", "-(3^(1/2)+2)"},
 {"-72",    "\x8A*2*(-5)^(-1)",  "",                                   "-(5/8+5^(1/2)/8)^(1/2)",             "-(5+2*5^(1/2))^(1/2)"},
 {"-135/2", "\x8A*(-3)*8^(-1)",  "",                                   "-(2+2^(1/2))^(1/2)*2^(-1)",          "-1-2^(1/2)"},
 {"-60",    "\x8A*(-3)^(-1)",    "",                                   "-3^(1/2)*2^(-1)",                    "-3^(1/2)"},
 {"-54",    "\x8A*(-3)*10^(-1)", "",                                   "4^(-1)*(-1-5^(1/2))",                "-(1+2*5^(-1/2))^(1/2)"},
 {"-45",    "\x8A*(-4)^(-1)",    "",                                   "(-1)*(2^(-1/2))",                    "-1"},
 {"-36",    "\x8A*(-5)^(-1)",    "",                                   "-(5/8-5^(1/2)/8)^(1/2)",             "-(5-2*5^(1/2))^(1/2)"},
 {"-30",    "\x8A*(-6)^(-1)",    "",                                   "-0.5",                               "-3^(-1/2)"},
 {"-45/2",  "\x8A*(-8)^(-1)",    "",                                   "(2-2^(1/2))^(1/2)*(-2)^(-1)",        "1-2^(1/2)"},
 {"-18",    "\x8A*(-10)^(-1)",   "",                                   "4^(-1)*(1-5^(1/2))",                 "-(1-2*5^(-1/2))^(1/2)"},
 {"-15",    "\x8A*(-12)^(-1)",   "",                                   "-6^(1/2)*4^(-1)+2^(1/2)*4^(-1)",     "3^(1/2)-2"},
 {"0",      "0",                 "1",                                  "0",                                  "0"},
 {"15",     "\x8A*12^(-1)",      "6^(1/2)*4^(-1)+2^(1/2)*4^(-1)",      "6^(1/2)*4^(-1)+2^(1/2)*(-4)^(-1)",   "-(3^(1/2)-2)"},
 {"18",     "\x8A*10^(-1)",      "(5/8+5^(1/2)/8)^(1/2)",              "4^(-1)*(5^(1/2)-1)",                 "(1-2*5^(-1/2))^(1/2)"},
 {"45/2",   "\x8A*8^(-1)",       "(2+2^(1/2))^(1/2)*2^(-1)",           "(2-2^(1/2))^(1/2)*2^(-1)",           "2^(1/2)-1"},
 {"30",     "\x8A*6^(-1)",       "3^(1/2)*2^(-1)",                     "0.5",                                "3^(-1/2)"},
 {"36",     "\x8A*5^(-1)",       "(5^(1/2)+1)*4^(-1)",                 "(5/8-5^(1/2)/8)^(1/2)",              "(5-2*5^(1/2))^(1/2)"},
 {"45",     "\x8A*4^(-1)",       "2^(-1/2)",                           "2^(-1/2)",                           "1"},
 {"54",     "\x8A*3*10^(-1)",    "(5/8-5^(1/2)/8)^(1/2)",              "4^(-1)*(5^(1/2)+1)",                 "(1+2*5^(-1/2))^(1/2)"},
 {"60",     "\x8A*3^(-1)",       "0.5",                                "3^(1/2)*2^(-1)",                     "3^(1/2)"},
 {"135/2",  "\x8A*3*8^(-1)",     "(2-2^(1/2))^(1/2)*2^(-1)",           "(2+2^(1/2))^(1/2)*2^(-1)",           "1+2^(1/2)"},
 {"72",     "\x8A*2*5^(-1)",     "(5^(1/2)-1)*4^(-1)",                 "(5/8+5^(1/2)/8)^(1/2)",              "(5+2*5^(1/2))^(1/2)"},
 {"75",     "\x8A*5*12^(-1)",    "6^(1/2)*4^(-1)+2^(1/2)*(-4)^(-1)",   "6^(1/2)*4^(-1)+2^(1/2)*4^(-1)",      "3^(1/2)+2"},
 {"90",     "\x8A*2^(-1)",       "0",                                  "1",                                  "undef"},
 {"105",    "\x8A*7*12^(-1)",    "-6^(1/2)*4^(-1)+2^(1/2)*4^(-1)",     "",                                   ""},
 {"108",    "\x8A*3*5^(-1)",     "(1-5^(1/2))*4^(-1)",                 "",                                   ""},
 {"225/2",  "\x8A*5*8^(-1)",     "(2-2^(1/2))^(1/2)*(-2)^(-1)",        "",                                   ""},
 {"120",    "\x8A*2*3^(-1)",     "-0.5",                               "",                                   ""},
 {"126",    "\x8A*7*10^(-1)",    "-(5*8^(-1)-5^(1/2)*8^(-1))^(1/2)",   "",                                   ""},
 {"135",    "\x8A*3*4^(-1)",     "(-1)*(2^(-1/2))",                    "",                                   ""},
 {"144",    "\x8A*4*5^(-1)",     "(-5^(1/2)-1)*4^(-1)",                "",                                   ""},
 {"150",    "\x8A*5*6^(-1)",     "-3^(1/2)*2^(-1)",                    "",                                   ""},
 {"315/2",  "\x8A*7*8^(-1)",     "-(2+2^(1/2))^(1/2)*2^(-1)",          "",                                   ""},
 {"162",    "\x8A*9*10^(-1)",    "-(5*8^(-1)+5^(1/2)*8^(-1))^(1/2)",   "",                                   ""},
 {"165",    "\x8A*11*12^(-1)",   "(-1)*6^(1/2)*4^(-1)-2^(1/2)*4^(-1)", "",                                   ""},
 {"180",    "\x8A",              "-1",                                 "0",                                  "0"}};

Expression * Trigonometry::table(const Expression * e, Expression::Type type, Context & context, Expression::AngleUnit angleUnit) {
  assert(type == Expression::Type::Sine || type == Expression::Type::Cosine || type == Expression::Type::Tangent || type == Expression::Type::ArcCosine || type == Expression::Type::ArcSine || type == Expression::Type::ArcTangent);
  int angleUnitIndex = angleUnit == Expression::AngleUnit::Radian ? 1 : 0;
  int trigonometricFunctionIndex = type == Expression::Type::Cosine || type == Expression::Type::ArcCosine ? 2 : (type == Expression::Type::Sine || type == Expression::Type::ArcSine ? 3 : 4);
  int inputIndex = type == Expression::Type::ArcCosine || type == Expression::Type::ArcSine || type == Expression::Type::ArcTangent ? trigonometricFunctionIndex : angleUnitIndex;
  int outputIndex = type == Expression::Type::ArcCosine || type == Expression::Type::ArcSine || type == Expression::Type::ArcTangent ? angleUnitIndex : trigonometricFunctionIndex;

  /* Avoid looping if we can exclude quickly that the e is in the table */
  if (inputIndex == 0 && e->type() != Expression::Type::Rational) {
    return nullptr;
  }
  if (inputIndex == 1 && e->type() != Expression::Type::Rational && e->type() != Expression::Type::Multiplication && e->type() != Expression::Type::Symbol) {
    return nullptr;
  }
  if (inputIndex >1 && e->type() != Expression::Type::Rational && e->type() != Expression::Type::Multiplication && e->type() != Expression::Type::Power && e->type() != Expression::Type::Addition) {
    return nullptr;
  }
  for (int i = 0; i < k_numberOfEntries; i++) {
    Expression * input = Expression::parse(cheatTable[i][inputIndex]);
    if (input == nullptr) {
      continue;
    }
    Expression::Reduce(&input, context, angleUnit);
    bool rightInput = input->isIdenticalTo(e);
    delete input;
    if (rightInput) {
      Expression * output = Expression::parse(cheatTable[i][outputIndex]);
      if (output == nullptr) {
        return nullptr;
      }
      Expression::Reduce(&output, context, angleUnit);
      return output;
    }
  }
  return nullptr;
}


template <typename T>
std::complex<T> Trigonometry::ConvertToRadian(const std::complex<T> c, Expression::AngleUnit angleUnit) {
  if (angleUnit == Expression::AngleUnit::Degree) {
    return c*std::complex<T>(M_PI/180.0);
  }
  return c;
}

template <typename T>
std::complex<T> Trigonometry::ConvertRadianToAngleUnit(const std::complex<T> c, Expression::AngleUnit angleUnit) {
  if (angleUnit == Expression::AngleUnit::Degree) {
    return c*std::complex<T>(180/M_PI);
  }
  return c;
}

template<typename T>
T Trigonometry::RoundToMeaningfulDigits(T f) {
  /* Cheat: openbsd trigonometric functions are numerical implementation and
   * thus are approximative.
   * The error epsilon is ~1E-7 on float and ~1E-15 on double. In order to
   * avoid weird results as acos(1) = 6E-17 or cos(Pi/2) = 4E-17, we keep only
   * 15 (or 7) decimals.
   * We can't do that for all evaluation as the user can operate on values as
   * small as 1E-308 (in double) and most results still be correct. */
  T precision = 10*Expression::epsilon<T>();
  return std::round(f/precision)*precision;
}

template <typename T>
std::complex<T> Trigonometry::RoundToMeaningfulDigits(const std::complex<T> c) {
  return std::complex<T>(RoundToMeaningfulDigits(c.real()), RoundToMeaningfulDigits(c.imag()));
}

template std::complex<float> Trigonometry::ConvertToRadian<float>(std::complex<float>, Expression::AngleUnit);
template std::complex<double> Trigonometry::ConvertToRadian<double>(std::complex<double>, Expression::AngleUnit);
template std::complex<float> Trigonometry::ConvertRadianToAngleUnit<float>(std::complex<float>, Expression::AngleUnit);
template std::complex<double> Trigonometry::ConvertRadianToAngleUnit<double>(std::complex<double>, Expression::AngleUnit);
template std::complex<float> Trigonometry::RoundToMeaningfulDigits<float>(std::complex<float>);
template std::complex<double> Trigonometry::RoundToMeaningfulDigits<double>(std::complex<double>);

}

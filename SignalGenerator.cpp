#include "SignalGenerator.h"
#include <cmath>
#include <algorithm>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

SignalGenerator::SignalGenerator()
    : type(Type::KROK),
      T(20),
      A(1.0),
      S(0.0),
      p(0.5)
{
}
double SignalGenerator::valueAtStep(int k) const
{
    if (T <= 0)
        return S;
    double wynik = S;
    if (T <= 0)
    {
        wynik = S;
    }
    else if (type == Type::SIN)
    {
        int modT = k % T;
        double angle = (static_cast<double>(modT) * 2.0 * M_PI) / static_cast<double>(T);
        wynik = A * std::sin(angle) + S;
    }
    else
    {
        int modT = k % T;
        double switchPoint = static_cast<double>(T) * p;
        if (static_cast<double>(modT) < switchPoint)
            return A + S;
        else
            return -A + S;
    }
    return wynik;
}
void SignalGenerator::setTempType(Type t_new) { Temptype = t_new; };

void SignalGenerator::setTempPeriod(int T_new)
{
    if (T_new < 1)
        TempT_new = 1;
    else
        TempT_new = T_new;
}

void SignalGenerator::setTempAmplitude(double A_new)
{
    if (A_new < 0.0)
        TempA_new = 0.0;
    else
        TempA_new = A_new;
}

void SignalGenerator::setTempOffset(double TempS_new) { TempS = TempS_new; }

void SignalGenerator::setTempDuty(double p_new)
{
    if (p_new < 0.0)
        Tempp_new = 0.0;
    else if (p_new > 1.0)
        Tempp_new = 1.0;
    else
        Tempp_new = p_new;
}

void SignalGenerator::resetMemory()
{
    type = Type::KROK;
    T = 20;
    A = 1.0;
    S = 0.0;
    p = 0.5;
}

void SignalGenerator::commitChanges()
{
    type = Temptype;
    T = TempT_new;
    A = TempA_new;
    S = TempS;
    p = Tempp_new;
}

void SignalGenerator::discardChanges()
{
    Temptype = type;
    TempT_new = T;
    TempA_new = A;
    TempS = S;
    Tempp_new = p;
}

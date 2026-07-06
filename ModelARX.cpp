#include "ModelARX.h"
#include <algorithm>
#include <cmath>
#include <stdexcept>

ModelARX::ModelARX(const QVector<double> &A_, const QVector<double> &B_,
                   int delay, double sigma_)
    : A(A_), B(B_), k(std::max(1, delay)), sigma(std::max(0.0, sigma_)),
      u_min(-10.0), u_max(10.0), y_min(-10.0), y_max(10.0), limitsEnabled(true),
      dist(0.0, sigma_ > 0.0 ? sigma_ : 1.0)
{

    y_buf.assign(A.size(), 0.0);
    u_buf.assign(B.size() + k, 0.0);

    gen.seed(std::random_device{}());

    tempA = A;
    tempB = B;
    tempK = k;
    tempSigma = sigma;
    tempU_min = u_min;
    tempU_max = u_max;
    tempY_min = y_min;
    tempY_max = y_max;
    tempLimitsEnabled = limitsEnabled;
}

double ModelARX::saturate(double v, double lo, double hi) const
{
    if (v < lo)
        return lo;
    if (v > hi)
        return hi;
    return v;
}

double ModelARX::symuluj(double u_in)
{
    double u_saturated = u_in;
    if (limitsEnabled)
    {
        u_saturated = saturate(u_in, u_min, u_max);
    }
    u_buf.push_front(u_saturated);
    if (u_buf.size() > B.size() + k)
    {
        u_buf.pop_back();
    }
    double y_new = 0.0;
    for (size_t i = 0; i < A.size(); ++i)
    {
        if (i < y_buf.size())
        {
            y_new -= A[i] * y_buf[i];
        }
    }
    for (size_t j = 0; j < B.size(); ++j)
    {
        size_t index = j + k;
        if (index < u_buf.size())
        {
            y_new += B[j] * u_buf[index];
        }
    }
    if (sigma > 0.0)
    {
        y_new += dist(gen);
    }
    if (limitsEnabled)
    {
        y_new = saturate(y_new, y_min, y_max);
    }
    y_buf.push_front(y_new);
    if (y_buf.size() > A.size())
    {
        y_buf.pop_back();
    }
    return y_new;
}

void ModelARX::resetMemory()
{
    if (!A.empty())
    {
        y_buf.assign(A.size(), 0.0);
    }
    else
    {
        std::fill(y_buf.begin(), y_buf.end(), 0.0);
    }

    std::fill(u_buf.begin(), u_buf.end(), 0.0);
}
void ModelARX::setTempA(const QVector<double> &A_new) { tempA = A_new; }
void ModelARX::setTempB(const QVector<double> &B_new) { tempB = B_new; }
void ModelARX::setTempDelay(int k_new) { tempK = std::max(0, k_new); }
void ModelARX::setTempSigma(double sigma_new) { tempSigma = std::max(0.0, sigma_new); }
void ModelARX::setTempControlLimits(double u_min_, double u_max_)
{
    tempU_min = std::min(u_min_, u_max_);
    tempU_max = std::max(u_min_, u_max_);
}
void ModelARX::setTempOutputLimits(double y_min_, double y_max_)
{
    tempY_min = std::min(y_min_, y_max_);
    tempY_max = std::max(y_min_, y_max_);
}

void ModelARX::setA(const QVector<double> &A_new) { A = A_new; }
void ModelARX::setB(const QVector<double> &B_new) { B = B_new; }
void ModelARX::setDelay(int k_new) { k = std::max(0, k_new); }
void ModelARX::setSigma(double sigma_new) { sigma = std::max(0.0, sigma_new); }

void ModelARX::setControlLimits(double u_min_, double u_max_)
{
    u_min = std::min(u_min_, u_max_);
    u_max = std::max(u_min_, u_max_);
}
void ModelARX::setOutputLimits(double y_min_, double y_max_)
{
    y_min = std::min(y_min_, y_max_);
    y_max = std::max(y_min_, y_max_);
}
void ModelARX::setLimitsEnabled(bool enabled)
{
    limitsEnabled = enabled;
}
void ModelARX::ensureMinCoeffsOnCommit()
{
    if (tempA.size() < 3)
        tempA.resize(3, 0.0);
    if (tempB.size() < 3)
        tempB.resize(3, 0.0);
}
void ModelARX::commitChanges()
{
    limitsEnabled = tempLimitsEnabled;

    A = tempA;
    B = tempB;
    k = std::max(1, tempK);
    sigma = std::max(0.0, tempSigma);
    u_min = tempU_min;
    u_max = tempU_max;
    y_min = tempY_min;
    y_max = tempY_max;
    y_buf.resize(A.size(), 0.0);
    u_buf.resize(B.size() + k, 0.0);

    dist = std::normal_distribution<double>(0.0, sigma > 0.0 ? sigma : 1.0);
}
void ModelARX::discardChanges()
{
    tempA = A;
    tempB = B;
    tempK = k;
    tempSigma = sigma;
    tempU_min = u_min;
    tempU_max = u_max;
    tempY_min = y_min;
    tempY_max = y_max;
    tempLimitsEnabled = limitsEnabled;
}

#pragma once
#include <QDataStream>
#include <vector>
#include <deque>
#include <random>
#include <QVector>

class ModelARX
{
public:
    ModelARX() = default;
    ModelARX(const QVector<double> &A_,
             const QVector<double> &B_,
             int delay = 1,
             double sigma_ = 0.0);
    double symuluj(double u_in);
    void resetMemory();
    QVector<double> getA() const { return A; }
    QVector<double> getB() const { return B; }
    int getDelay() const { return k; }
    double getSigma() const { return sigma; }
    double getUMin() const { return u_min; }
    double getUMax() const { return u_max; }
    double getYMin() const { return y_min; }
    double getYMax() const { return y_max; }
    bool getLimitsEnabled() const { return limitsEnabled; }
    void setTempA(const QVector<double> &A_new);
    void setTempB(const QVector<double> &B_new);
    void setTempDelay(int k_new);
    void setTempSigma(double sigma_new);
    void setTempLimitsEnabled(bool enabled) { tempLimitsEnabled = enabled; }
    void setTempControlLimits(double u_min_, double u_max_);
    void setTempOutputLimits(double y_min_, double y_max_);

    void setA(const QVector<double> &A_new);
    void setB(const QVector<double> &B_new);
    void setDelay(int k_new);
    void setSigma(double sigma_new);
    void setControlLimits(double u_min_, double u_max_);
    void setOutputLimits(double y_min_, double y_max_);
    void commitChanges();
    void discardChanges();

    double saturate(double v, double lo, double hi) const;
    void setLimitsEnabled(bool enabled);
    bool areLimitsEnabled() const { return limitsEnabled; }

private:
    QVector<double> A;
    QVector<double> B;
    int k;
    double sigma;
    double u_min, u_max;
    double y_min, y_max;
    bool limitsEnabled;
    std::deque<double> y_buf;
    std::deque<double> u_buf;
    std::default_random_engine gen;
    std::normal_distribution<double> dist;
    QVector<double> tempA;
    QVector<double> tempB;
    int tempK;
    double tempSigma;
    double tempU_min, tempU_max;
    double tempY_min, tempY_max;
    bool tempLimitsEnabled;
    void ensureMinCoeffsOnCommit();
};

inline QDataStream &operator<<(QDataStream &out, const ModelARX &arx)
{
    out << arx.getA() << arx.getB() << arx.getDelay() << arx.getSigma() << arx.getUMin() << arx.getUMax() << arx.getYMin() << arx.getYMax() << arx.getLimitsEnabled();
    return out;
}

inline QDataStream &operator>>(QDataStream &in, ModelARX &arx)
{
    QVector<double> _A, _B;
    int _k;
    double _sigma, _umin, _umax, _ymin, _ymax;
    bool _limEnabled;
    in >> _A >> _B >> _k >> _sigma >> _umin >> _umax >> _ymin >> _ymax >> _limEnabled;
    arx.setA(_A);
    arx.setB(_B);
    arx.setDelay(_k);
    arx.setSigma(_sigma);
    arx.setControlLimits(_umin, _umax);
    arx.setOutputLimits(_ymin, _ymax);

    return in;
}

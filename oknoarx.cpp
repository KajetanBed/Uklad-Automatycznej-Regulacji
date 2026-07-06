#include "oknoarx.h"
#include "ui_oknoarx.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
oknoARX::oknoARX(QWidget *parent) : QDialog(parent),
                                    ui(new Ui::oknoARX)
{
    ui->setupUi(this);
    ui->scrollAreaA->setWidgetResizable(true);
    ui->scrollAreaB->setWidgetResizable(true);
    ui->scrollAreaA->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::MinimumExpanding);
    ui->scrollAreaB->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::MinimumExpanding);
    ui->limityFrame->setEnabled(ui->limityCheckBox->isChecked());
    connect(ui->limityCheckBox, &QCheckBox::toggled, this, [this](bool checked)
            { ui->limityFrame->setEnabled(checked); });
}
oknoARX::~oknoARX()
{
    delete ui;
}

QVector<double> oknoARX::getWielomianA() const
{
    QVector<double> wynik;
    for (auto *box : polaWspolczynnikowA)
    {
        wynik.push_back(box->value());
    }
    return wynik;
}

QVector<double> oknoARX::getWielomianB() const
{
    QVector<double> wynik;
    for (auto *box : polaWspolczynnikowB)
    {
        wynik.push_back(box->value());
    }
    return wynik;
}

double oknoARX::getOdchylenie() const
{
    return ui->odchylenieSpinBox->value();
}

int oknoARX::getOpoznienie() const
{
    return ui->probkiOpoznieniaspinBox->value();
}
void oknoARX::ustawDane(double odch, int opoz, QVector<double> A, QVector<double> B, double uMin, double uMax, double yMin, double yMax, bool enabled)
{
    ui->odchylenieSpinBox->setValue(odch);
    ui->probkiOpoznieniaspinBox->setValue(opoz);
    ui->uMaxSpinBox->setValue(uMax);
    ui->yMaxSpinBox->setValue(yMax);
    ui->uMinSpinBox->setValue(std::min(uMax, uMin));
    ui->yMinSpinBox->setValue(std::min(yMax, yMin));
    ui->limityCheckBox->setChecked(enabled);
    if (ui->limityFrame)
    {
        ui->limityFrame->setEnabled(enabled);
    }
    while (A.size() < 3)
        A.push_back(0.0);
    while (B.size() < 3)
        B.push_back(0.0);
    QWidget *nowyKontenerA = new QWidget();
    QVBoxLayout *nowyLayoutA = new QVBoxLayout(nowyKontenerA);
    nowyLayoutA->setAlignment(Qt::AlignTop);
    ui->scrollAreaA->setWidget(nowyKontenerA);
    ui->scrollAreaA->setWidgetResizable(true);

    QWidget *nowyKontenerB = new QWidget();
    QVBoxLayout *nowyLayoutB = new QVBoxLayout(nowyKontenerB);
    nowyLayoutB->setAlignment(Qt::AlignTop);
    ui->scrollAreaB->setWidget(nowyKontenerB);
    ui->scrollAreaB->setWidgetResizable(true);
    polaWspolczynnikowA.clear();
    polaWspolczynnikowB.clear();
    zbudujListePola(A, &polaWspolczynnikowA, ui->scrollAreaA);
    zbudujListePola(B, &polaWspolczynnikowB, ui->scrollAreaB);
}
void oknoARX::zbudujListePola(const QVector<double> &wektor, QList<QDoubleSpinBox *> *lista, QScrollArea *cel)
{
    QWidget *kontener = cel->widget();
    if (!kontener)
        return;
    QVBoxLayout *layout = qobject_cast<QVBoxLayout *>(kontener->layout());
    if (!layout)
        return;
    for (double wartosc : wektor)
    {
        QDoubleSpinBox *spin = new QDoubleSpinBox();
        spin->setRange(-2.0, 2.0);
        spin->setSingleStep(0.1);
        spin->setValue(wartosc);

        layout->addWidget(spin);
        lista->append(spin);
    }
    QPushButton *btnAdd = new QPushButton("Dodaj współczynnik");
    layout->addWidget(btnAdd);
    connect(btnAdd, &QPushButton::clicked, [=]()
            {
        QDoubleSpinBox *spin = new QDoubleSpinBox();
        spin->setRange(-2.0, 2.0);
        spin->setSingleStep(0.1);
        spin->setValue(0.0);
        QVBoxLayout *layoutPrzycisku = qobject_cast<QVBoxLayout*>(btnAdd->parentWidget()->layout());

        if (layoutPrzycisku) {
            int index = layoutPrzycisku->indexOf(btnAdd);
            layoutPrzycisku->insertWidget(index, spin);
            lista->append(spin);
        } });
}
void oknoARX::wyczyscDynamicznePola()
{
    for (auto *box : polaWspolczynnikowA)
    {
        delete box;
    }
    polaWspolczynnikowA.clear();

    for (auto *box : polaWspolczynnikowB)
    {
        delete box;
    }
    polaWspolczynnikowB.clear();
}
void oknoARX::on_buttonZapiszARX_clicked()
{
    accept();
}
void oknoARX::on_buttonAnulujARX_clicked()
{
    reject();
}
double oknoARX::getUMin() const { return ui->uMinSpinBox->value(); }
double oknoARX::getUMax() const { return ui->uMaxSpinBox->value(); }
double oknoARX::getYMin() const { return ui->yMinSpinBox->value(); }
double oknoARX::getYMax() const { return ui->yMaxSpinBox->value(); }
bool oknoARX::getLimitsEnabled() const { return ui->limityCheckBox->isChecked(); }

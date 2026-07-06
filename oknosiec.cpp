#include "oknosiec.h"
#include "ui_oknosiec.h"

oknosiec::oknosiec(QWidget *parent) : QDialog(parent),
                                      ui(new Ui::oknosiec)
{
    ui->setupUi(this);

    this->setWindowTitle("Konfiguracja połączenia sieciowego");
}

oknosiec::~oknosiec()
{
    delete ui;
}
QString oknosiec::getIP() const
{
    return ui->lineEditIP->text();
}
bool oknosiec::jestRegulatorem() const
{
    return ui->radioButtonRegulator->isChecked();
}

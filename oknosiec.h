#ifndef OKNOSIEC_H
#define OKNOSIEC_H

#include <QDialog>
namespace Ui
{
    class oknosiec;
}

class oknosiec : public QDialog
{
    Q_OBJECT

public:
    explicit oknosiec(QWidget *parent = nullptr);
    ~oknosiec();
    QString getIP() const;
    bool jestRegulatorem() const;

private:
    Ui::oknosiec *ui;
};

#endif // OKNOSIEC_H

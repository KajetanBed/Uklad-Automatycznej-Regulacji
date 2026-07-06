#ifndef KLIENT_H
#define KLIENT_H

#include "RegulatorPID.h"
#include "ModelARX.h"
#include <QObject>
#include <QTcpSocket>
#include <QDateTime>

class Klient : public QObject
{
    Q_OBJECT
public:
    explicit Klient(QObject *parent = nullptr);

    void connectTo(QString address, int port);
    void disconnectFrom();
    bool isConnected() { return _socket.isOpen(); }
    void sendMessage(QByteArray msg);
    bool czekajNaPolaczenie(int timeoutMs) { return _socket.waitForConnected(timeoutMs); }

signals:
    void connected(QString adr, int port);
    void disconnected();
    void odebranoSterowanie(quint32 idProbki, double u, double w);
    void odebranoWyjscie(quint32 idProbki, double y);
    void odebranoKonfiguracjeJSON(QByteArray json);
    void odebranoKomendeSterujaca(quint8 akcja, quint32 interwal);
    void odebranoHandshake(QString rola, QString adresIP);

    void odebranoARX(QByteArray json);

private slots:
    void slot_connected();
    void slot_readyRead();

private:
    void przetworzRamke(quint8 typ, quint32 id, QByteArray payload);

    QTcpSocket _socket;
    QString _ipAddress = "127.0.0.1";
    int _port = 12345;
    QByteArray _bufor;
};

#endif

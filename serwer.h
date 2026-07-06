#ifndef SERWER_H
#define SERWER_H

#include "RegulatorPID.h"
#include "ModelARX.h"
#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QDateTime>

class Serwer : public QObject
{
    Q_OBJECT
public:
    explicit Serwer(QObject *parent = nullptr);

    bool startListening(int port);
    bool isListening() { return _isListening; }
    void stopListening();
    void sendMessage(QByteArray msg);
    bool czekajNaKlienta(int timeout);
    QTcpSocket *pobierzPolaczenie();

signals:
    void newClientConnected(QString adr);
    void clientDisconnected();
    void newMsg(QByteArray);
    void odebranoSterowanie(quint32 idProbki, double u, double w);
    void odebranoWyjscie(quint32 idProbki, double y);
    void odebranoKonfiguracjeJSON(QByteArray json);
    void odebranoKomendeSterujaca(quint8 akcja, quint32 interwal);
    void odebranoHandshake(QString rola, QString adresIP);

    void odebranoARX(QByteArray json);

private slots:
    void slot_new_client();
    void slot_client_disconnected();
    void slot_newMsg();

private:
    void przetworzRamke(quint8 typ, quint32 id, QByteArray payload);

    bool _isListening = false;
    int _port = 12345;
    QTcpServer _server;
    QTcpSocket *_client = nullptr;
    QByteArray _bufor;
};

#endif

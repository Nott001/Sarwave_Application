#pragma once
#include <QObject>
#include <QString>
#include <QTcpSocket>

class NetworkManager : public QObject {
    Q_OBJECT
public:
    explicit NetworkManager(QObject *parent = nullptr);
    void connectToHost(const QString &host, quint16 port);
    void sendData(const QByteArray &data);
signals:
    void dataReceived(const QByteArray &data);
    void connected();
    void disconnected();
private:
    QTcpSocket *m_socket;
};

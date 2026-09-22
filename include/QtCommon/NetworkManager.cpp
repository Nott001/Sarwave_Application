#include "NetworkManager.h"

#include <QTcpSocket>

NetworkManager::NetworkManager(QObject* parent) : QObject(parent), m_socket(new QTcpSocket(this))
{
}

void NetworkManager::connectToHost(const QString& host, quint16 port)
{
    m_socket->connectToHost(host, port);
}

void NetworkManager::sendData(const QByteArray& data)
{
    m_socket->write(data);
}

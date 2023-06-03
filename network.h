#ifndef NETWORK_H
#define NETWORK_H

#include <QTcpServer>
#include <QSslSocket>
#include <QTcpSocket>

class Network: public QObject {
    Q_OBJECT

public:
    Network();
    void connectTo(QString, int, QString);
    void disconnect();
    void move(float, float, float, float);
    void sendToServer(int);
    void getAnswer();
    QMap<int, QPair<QString, QVector<float>>> getUsers() {return users;}
    QVector<QPair<QPair<int, int>, QVector<float>>> mapCoords;
    QMap<QString, QString> pictures;

signals:
    void needToPaint();
    void sendMapCoords();
    void sendPictures();

public slots:
    void tcpReady();
    void sslError();
    void TCPError(QAbstractSocket::SocketError error);

private:
    //QTcpSocket *socket;
    QSslSocket client_socket;
    QByteArray data;
    QString ip;
    QString nickname;
    int port;
    float x, y, speed_x, speed_y;

    QHash<QTcpSocket*, QByteArray*> buffers;
    QHash<QTcpSocket*, qint32*> sizes;
    QList<QSslError> err;
    QMap<int, QPair<QString, QVector<float>>> users;

};

#endif // NETWORK_H

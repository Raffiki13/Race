#ifndef SERVER_H
#define SERVER_H

#include <QTcpSocket>
#include <QTcpServer>
#include <QTableWidget>
#include <QWidget>
#include <QSslSocket>
#include <QMap>

QT_BEGIN_NAMESPACE
class QSessionManager;
QT_END_NAMESPACE

class Server : public QTcpServer {
    Q_OBJECT

public:
    explicit Server(QObject *parent = 0);
    //~Server();
    QSslSocket *socket;
    QTcpServer *server;
    QWidget *win;

private:
    void SendToClient(int, int);
    void window();
    void getRequest(int);
    void changeJournal();
    void collision();
    void xml();

    QVector<QSslSocket*> sockets;
    QByteArray data;
    QByteArray map;
    QTableWidget *tableWidget;
    QList<QSslError> err;

    QHash<QSslSocket*, QByteArray*> buffers;
    QVector<bool> connections;
    QHash<QSslSocket*, qint32*> sizes;
    QMap<QSslSocket*, QPair<QString, QVector<float>>> users;

public slots:
    void newConnectionRecognized();
    void showErrors();
    void incomingConnection(qintptr dsescriptor);
    void ready();
    void read();
    void sendDataAboutPlayers();
//    void sslError(QList<QSslError> errors) {}
};

#endif // SERVER_H

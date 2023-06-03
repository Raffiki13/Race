#ifndef GAME_H
#define GAME_H

#include <QWidget>
#include <QPainterPath>
#include <QDebug>
#include <QTcpSocket>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QDialog>
#include <QLineEdit>
#include <QComboBox>
#include <QLabel>
#include <QNetworkSession>

class Game : public QDialog {
    Q_OBJECT

public:
    Game();
    void setCar();
    bool moveUp;
    bool moveDown;
    bool moveRight;
    bool moveLeft;
    bool direct;
    QVector<QPair<QPair<int, int>, QVector<float>>> mapCoords;
    QMap<QString, QString> pictures;

    float getCoordX() {return x;}
    float getCoordY() {return y;}
    float getSpeedX() {return speed_x;}
    float getSpeedY() {return speed_y;}

    void updateField();
    void drawCars(QPainter &);
    void setUsers(QMap<int, QPair<QString, QVector<float>>> tmp) {users = tmp;}
    void setNickname(QString tmp) {nickname = tmp;}

signals:
    void signal();
    void disc();

public slots:
    void moveCar();
    void speedUp();
    void speedDown();
    void sendDataToServer();

private:
    QImage img;
    QPainter paint;
    QString nickname;    
    QPainterPath car;
    QPainterPath field;
    QVector<QPainterPath> cars;
    QMap<int, QPair<QString, QVector<float>>> users;

    int width_road;
    float speed;
    float alpha;
    float speed_d;
    float speed_x;
    float speed_y;
    float alpha_step;
    float speed_dx;
    float speed_dy;
    bool speed_up;
    bool speed_down;
    bool checkSpace;
    bool checkShift;

    void paintEvent(QPaintEvent*);
    void setField();
    void drawField(QPainter&);

protected:
    void closeEvent(QCloseEvent*) override;
    void keyPressEvent(QKeyEvent*);
    void keyReleaseEvent(QKeyEvent*);
    float x = 0;
    float y = 0;
    int center_x_window;
    int center_y_window;

    QPainter* my_painter = nullptr;   
};

#endif // GAME_H

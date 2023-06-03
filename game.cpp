#include "game.h"
#include <iostream>
#include <QPainter>
#include <QKeyEvent>
#include <QTimer>
#include <QtMath>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMenu>
#include <QMenuBar>
#include <QPushButton>
#include <QHostInfo>
#include <QNetworkConfigurationManager>
#include <QDialogButtonBox>
#include <QSettings>
#include <QGuiApplication>
#include <QStyleHints>
#include <QGroupBox>

Game::Game() {
    Qt::WindowFlags flags = Qt::Dialog | Qt::WindowMaximizeButtonHint| Qt::WindowCloseButtonHint | Qt::WindowMinimizeButtonHint;
    setWindowFlags(flags);
    setFixedHeight(1200);
    setFixedWidth(900);

    width_road = 100;
    setField();
    QTimer* move_the_car = new QTimer();
    QTimer* timer = new QTimer();
    QTimer* startMoving = new QTimer();
    QTimer* speedLow = new QTimer();
    connect(move_the_car, SIGNAL(timeout()), this, SLOT(moveCar()));
    connect(timer, SIGNAL(timeout()), this, SLOT(sendDataToServer()));
    connect(startMoving, SIGNAL(timeout()), this, SLOT(speedUp()));
    connect(speedLow, SIGNAL(timeout()), this, SLOT(speedDown()));
    move_the_car->start(50);
    timer->start(50);
    startMoving->start(50);
    speedLow->start(125);

    speed = 0;
    alpha = 0;
    alpha_step = 5;
    speed_x = 0;
    speed_y = 0;
    x = 30;
    y = 30;
    moveUp = false;
    moveDown = false;
    moveRight = false;
    moveLeft = false;
    direct = false;
    speed_d = false;
    checkSpace = false;
    checkShift = false;
    speed_up = false;
    speed_down = false;
}

void Game::closeEvent(QCloseEvent *event) {
    emit disc();
}

void Game::sendDataToServer() {
    emit signal();
}

void Game::updateField() {
    foreach (int key, users.keys()) {
        if (nickname == users[key].first) {
            x = users[key].second[0];
            y = users[key].second[1];
            speed_x = users[key].second[2];
            speed_y = users[key].second[3];
        }
    }
    update();
}

void Game::setField() {
    QPainterPath path;

    for (int i = 0; i < mapCoords.size(); i++){
        if (mapCoords[i].first.second == 1){
            path.moveTo(mapCoords[i].second[0],mapCoords[i].second[1]);
        }
        if (mapCoords[i].first.second == 2){
            path.lineTo(mapCoords[i].second[0],mapCoords[i].second[1]);
        }
    }
    this->field = path;
    update();
}

void Game::setCar() {
        QPainterPath car;
        car.addRoundedRect(0, 0, 40,  20, 3, 3);
        this->car = car;
        update();
}

void Game::drawCars(QPainter &painter) {
    foreach (int key, users.keys()) {
        img.load("/home/mikaraf/LAB/1race/laba/Game_2/images/car"+pictures[users[key].first]);
        painter.resetTransform();
        QMatrix rot;
        rot.rotate(qRadiansToDegrees(atan2(users[key].second[3], users[key].second[2]))+90);
        QImage out = img.transformed(rot);
        painter.drawImage(users[key].second[0], users[key].second[1], out);
    }
}

void Game::drawField(QPainter &painter) {
    setField();
    painter.resetTransform();
    painter.drawPath(field);
}

void Game::paintEvent(QPaintEvent *event) {
    QPainter painter(this);
    my_painter  = &painter;
    painter.setRenderHint(QPainter::Antialiasing);
    painter.save();
    drawField(painter);
    drawCars(painter);
    painter.restore();
}

void Game::keyPressEvent(QKeyEvent *event) {
    if (event->key() == Qt::Key_Space) {
        speed_down = true;
    }
    if (event->key() == Qt::Key_Up && event->modifiers() == Qt::ShiftModifier) {
        moveUp = true;
        moveDown = false;
        speed_up = true;
    }
    if (event->key() == Qt::Key_Left && event->modifiers() == Qt::ShiftModifier) {
        moveRight = false;
        moveLeft = true;
        speed_up = true;
    }
    if (event->key() == Qt::Key_Down && event->modifiers() == Qt::ShiftModifier) {
        moveDown = true;
        moveUp = false;
        speed_up = true;
    }
    if (event->key() == Qt::Key_Right && event->modifiers() == Qt::ShiftModifier) {
        moveRight = true;
        moveLeft = false;
        speed_up = true;
    }

    if (event->key() == Qt::Key_Up) {
        moveUp = true;
        moveDown = false;
    }
    if (event->key() == Qt::Key_Left) {
        moveRight = false;
        moveLeft = true;
    }
    if (event->key() == Qt::Key_Down) {
        moveDown = true;
        moveUp = false;
    }
    if (event->key() == Qt::Key_Right) {
        moveRight = true;
        moveLeft = false;
    }
    moveCar();
}

void Game::moveCar() {
    speed_x = qCos(qDegreesToRadians(alpha)) * speed;
    speed_y = qSin(qDegreesToRadians(alpha)) * speed;

    if (speed_down && !checkSpace) {
        speed /= 2;
        checkSpace = true;
    } else if (!speed_down && checkSpace) {
        speed *= 2;
        checkSpace = false;
    }

    if (speed_up && !checkShift) {
        speed *= 2;
        checkShift = true;
    } else if (!speed_up && checkShift) {
        speed /= 2;
        checkShift = false;
    }

    if (moveUp && moveLeft) {
        x +=  speed_x;
        y -= speed_y;
        alpha += alpha_step;
    } else if (moveUp && moveRight) {
        x +=  speed_x;
        y -= speed_y;
        alpha -= alpha_step;
    } else if (moveDown && moveLeft) {
        x -=  speed_x;
        y += speed_y;
        alpha -= alpha_step;
    } else if (moveDown && moveRight) {
        x -=  speed_x;
        y += speed_y;
        alpha += alpha_step;
    } else if (moveUp) {
        x +=  speed_x;
        y -= speed_y;
    } else if (moveDown) {
        x -=  speed_x;
        y += speed_y;
    } else if (direct && moveRight) {
        x +=  speed_x;
        y -= speed_y;
        alpha -= alpha_step;
    } else if (!direct && moveLeft) {
        x -=  speed_x;
        y += speed_y;
        alpha -= alpha_step;
    } else if (direct && moveLeft) {
        x +=  speed_x;
        y -= speed_y;
        alpha += alpha_step;
    } else if (direct && moveRight) {
        x -=  speed_x;
        y += speed_y;
        alpha += alpha_step;
    }

    emit sendDataToServer();
    update();
}

void Game::keyReleaseEvent(QKeyEvent *event) {
    if (event->key() == Qt::Key_Space)
        speed_down = false;
    if (event->key() == Qt::Key_Shift)
        speed_up = false;
    if (event->key() == Qt::Key_Up){
        moveUp = false;
        direct = true;
    }
    if (event->key() == Qt::Key_Left){
        moveLeft = false;
    }
    if (event->key() == Qt::Key_Down){
        moveDown = false;
        direct = false;
    }
    if (event->key() == Qt::Key_Right){
        moveRight = false;
    }
}

void Game::speedDown() {
    if (speed > 0 && !moveUp && !moveDown  && !checkSpace) {
        speed -= speed_d;
        speed_x = qCos(qDegreesToRadians(alpha)) * speed;
        speed_y = qSin(qDegreesToRadians(alpha)) * speed;
        emit signal();
    }
}

void Game::speedUp(){
    if (speed < 5 && (moveUp || moveDown) && !checkSpace) {
        speed += speed_d;
    }
}

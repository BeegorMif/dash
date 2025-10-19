#pragma once

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QTimer>
#include <QVBoxLayout>

class ShutdownPage : public QWidget
{
    Q_OBJECT
public:
    explicit ShutdownPage(QWidget *parent = nullptr);
    void startCountdown(int seconds = 10);

signals:
    void cancelled();
    void countdownFinished();

private slots:
    void updateCountdown();

private:
    QLabel *labelStatus;
    QLabel *labelCountdown;
    QPushButton *buttonCancel;
    QTimer *timer;
    int timeLeft;
};

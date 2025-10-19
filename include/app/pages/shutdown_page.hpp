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

signals:
    void cancelled();
    void countdownFinished();

public slots:
    void startCountdown(int seconds = 30);
    void cancel();

private slots:
    void updateCountdown();

private:
    QLabel *label;
    QPushButton *cancelButton;
    QTimer *timer;
    int remaining;
};

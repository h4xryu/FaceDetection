#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtWidgets>
#include <iostream>
#include <QQueue>

class QTimer;
class QFile;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    void updateGraph(); // 그래프 업데이트

    ~MainWindow();

private slots:

    void on_pushButton_Start_clicked();
    void on_pushButton_Stop_clicked();

private:
    Ui::MainWindow *ui;
    QTimer* DataTimer;
    QFile* file;
    QTextStream* in;
    int linesProcessed; // 처리 중인 라인
    int xValue;
    int frameProcessed;  // 처리 중인 프레임

    void setupGraph();
};

#endif // MAINWINDOW_H

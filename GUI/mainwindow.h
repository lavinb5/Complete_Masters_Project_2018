#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "thread.h"
#include "gridcoords.h"

#include <QtCore>
#include <QtGui>
#include <QMouseEvent>
#include <QDebug>
#include <QMessageBox>
#include <unistd.h>

#define ADDRESS     "tcp://10.10.101.9"
#define CLIENTID    "PubQt"
#define AUTHMETHOD  "lavinb"
#define AUTHTOKEN   "pass"
#define TOPIC       "test"
#define QOS         2
#define TIMEOUT     10000L

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    // cooridnates
    int gridwidth, gridheight;
    void setColWidth(int col);
    int getColWidth();
    void setRowWidth(int row);
    int getRowWidth();
    bool leftpressed, rightpressed;
    QPoint MainPoints, startPoint, endPoint;
    QPoint grid_startPoint, grid_endPoint;
    bool startlocation, endlocation;
    bool startlocationSet, endlocationSet;
    int obstaclecount;
    bool selectobstacle, obstaclesSet;
    QPoint obstaclePoint, obstacles[100], grid_obstacles[100];
    QPoint gridPath[100], mainPath[100];
    int traverseInc;
    bool pathset, path, firstpathDraw;
    int path_points_total;
    QPoint gridTraverse[100], mainTraverse[100];
    QPoint currentLoc;
    bool robotTraversing;

    //testing code
    bool testing;
    int testArray[100] = {0};
    int testnum;

private slots:
    void on_start_loc_Button_clicked();
    void on_end_loc_Button_clicked();
    void on_start_obstacle_Button_clicked();
    void on_set_obs_Button_clicked();
    void on_path_plan_Button_clicked();
    void received_frame(unsigned char, int, unsigned char*);
signals:
    //testing code
    void testinc();

protected:
    void paintEvent(QPaintEvent *e);
    void mousePressEvent(QMouseEvent *mev);

private:
    Ui::MainWindow *ui;
    int colwidth;
    int rowwidth;
    Thread *thread;
    void send_mqtt_frame(unsigned char, int, unsigned char*);
    //testing code
    QElapsedTimer timer;
};

#endif // MAINWINDOW_H

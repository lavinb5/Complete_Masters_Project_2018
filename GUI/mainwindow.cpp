#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QtCore>


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->debug_Button->setVisible(false); // this button is used for debugging
    QString style1 = "border-style: outset;\
            border-width: 2px;\
            border-radius: 10px;\
            border-color: blue;\
            font: bold 12px;\
            padding: 6px;";
    ui->path_plan_Button->setStyleSheet(style1);
    ui->start_loc_Button->setStyleSheet(style1 + "background-color: rgb(153, 255, 153);");
    ui->start_obstacle_Button->setStyleSheet(style1);
    ui->set_obs_Button->setStyleSheet(style1);
    ui->end_loc_Button->setStyleSheet(style1 + "background-color: rgb(153, 204, 255);");
    ui->debug_Button->setStyleSheet(style1);
    QString style2 = "font: bold 12px;";
    ui->label->setStyleSheet(style2);
    ui->label_2->setStyleSheet(style2);
    ui->label_3->setStyleSheet(style2);
    ui->obstacle_count_label->setStyleSheet(style2);
    QString style3 = "border-style: outset;\
            border-width: 2px;\
            border-radius: 5px;\
            border-color: blue;\
            font: bold 12px;";
    ui->message_textbox->setStyleSheet(style3);

    leftpressed = false;
    rightpressed = false;

    gridwidth = 0;
    gridheight = 0;

    startlocation = false;
    startlocationSet = false;
    endlocation = false;
    endlocationSet = false;
    obstaclecount = 0;
    selectobstacle = false;
    obstaclesSet = false;
    pathset = false;
    path = false;
    firstpathDraw = false;
    path_points_total = 0;
    traverseInc = 0;
    robotTraversing = false;

    ui->message_textbox->insertPlainText("Plese select start, finish and obstacle locations");

    thread = new Thread(this);
    QObject::connect(thread, SIGNAL(mqqt_frame_pass(unsigned char,int,unsigned char*)),this,SLOT(received_frame(unsigned char,int,unsigned char*)));
    thread->start();

    //testing code
    testing = false;
    testnum = 0;
    connect(this, SIGNAL(testinc()),this, SLOT(on_path_plan_Button_clicked()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::paintEvent(QPaintEvent *e)
{
    QPainter painter(this);

    QPen redpen(Qt::red);
    redpen.setWidth(2);

    QPen blackpen(Qt::black);
    blackpen.setWidth(1);

    QPen obstaclepen(Qt::black);
    obstaclepen.setWidth(25);

    QPen pathpen(QColor(235,205,9));
    pathpen.setWidth(4);

    QBrush whitebrush(Qt::white);
    QBrush greenbrush(Qt::green);
    QBrush bluebrush(Qt::blue);

    QPen bluepen(Qt::blue);
    bluepen.setWidth(1);

    int startx = 10, starty = 10;
    int endx = 400 + startx, endy = 400 + starty;
    int framewidth = endx-startx, frameheight = endy-starty;
    gridwidth = framewidth;

    gridheight = frameheight;

    QRect frame(startx, starty, framewidth, frameheight);

    painter.fillRect(frame, whitebrush);
    painter.setPen(redpen);
    painter.drawRect(frame);

    painter.setPen(redpen);
    // draw columns
    int colnum = 10;
    int gridColwidth = framewidth/colnum;

    this->setColWidth(gridColwidth );

    QPoint columnstart, columnend;
    for(int i=1; i<colnum; i++){
        columnstart.setX(startx + gridColwidth*i);
        columnstart.setY(starty);
        columnend.setX(startx + gridColwidth*i);
        columnend.setY(endy);

        painter.drawLine(columnstart, columnend);
    }
    // draw rows
    int rownum = 10;
    int gridRowwidth = frameheight/rownum;
    this->setRowWidth(gridRowwidth );
    QPoint rowstart, rowend;
    for(int i=1; i<rownum; i++){
        rowstart.setX(startx);
        rowstart.setY(starty+ gridRowwidth*i);
        rowend.setX(endx);
        rowend.setY(starty + gridRowwidth*i);

        painter.drawLine(rowstart, rowend);
    }

    if(pathset == true || path == true)
    {
        qDebug() << "Pathing path";
        // draw path
        painter.setPen(pathpen);
        int nextpt;
        //painter.drawLine(mainPath[0], mainPath[2]);
        for(int i=0; i<(path_points_total-1); i++)
        {
            nextpt = i+1;
            painter.drawLine(mainPath[i], mainPath[nextpt]);
        }
        pathset = true;
        path = false;
        // GUI send ack to rpi first time drawing the path
        // use flag
        // when rpi acks the path ack from gui chnage flag state
        if(firstpathDraw == true)
        {
            // send ack
            qDebug() << "Frist time drawing path";
            firstpathDraw = false;
            unsigned char pay[] = {'p'};
            int pay_len = sizeof(pay)/sizeof(pay[0]);
            unsigned char frametype = 'a';  // send obstacle frame

            //testing code
            //***comment out for testing the algorithm
            send_mqtt_frame(frametype, pay_len, pay);
        }
    }

    if(leftpressed == true || rightpressed == true || pathset == true || path == true){
        qDebug() << "main paint";
        leftpressed = false;
        rightpressed = false;
        if((startlocation == true) || (startlocationSet == true))
        {
            painter.setBrush(greenbrush);
            painter.setPen(blackpen);
            painter.drawEllipse(startPoint, 10,10);
            startlocation = false;
            startlocationSet = true;
        }
        if((endlocation == true) || (endlocationSet == true))
        {
            painter.setBrush(bluebrush);
            painter.setPen(blackpen);
            painter.drawEllipse(endPoint, 10,10);
            endlocation = false;
            endlocationSet = true;
        }
        if((selectobstacle == true) || (obstaclesSet == true))
        {
            painter.setPen(obstaclepen);
            for(int i=0; i<obstaclecount; i++)
            {
                painter.drawPoint(obstacles[i]);
            }
        }
    }

    if(robotTraversing == true)
    {
        GridCoords *mainC = new GridCoords();
        for(int i=0; i<traverseInc; i++)
        {
            mainC->setMainCoord(10, this->getColWidth(), gridTraverse[i].x(), this->getRowWidth(), gridTraverse[i].y());
            mainTraverse[i] = mainC->getMainCoord();
            painter.setBrush(greenbrush);
            painter.setPen(blackpen);
            painter.drawEllipse(mainTraverse[i], 5,5);
        }
    }

    //testing code
//    if(testing == true){
//        emit(testinc());
//    }


}

void MainWindow::setColWidth(int col)
{
    colwidth = col;
}

int MainWindow::getColWidth()
{
    return colwidth;
}

void MainWindow::setRowWidth(int row)
{
    rowwidth = row;
}

int MainWindow::getRowWidth()
{
    return rowwidth;
}

void MainWindow::mousePressEvent(QMouseEvent *mev){

    //qDebug() << "Grid width: " << gridwidth;

    if(mev->x() > gridwidth || mev->x() < 10 || mev->y() > gridheight || mev->y() < 10){
        qDebug() << "Click outside frame";
    }
    else
    {
        GridCoords *gridcoord = new GridCoords();
        gridcoord->setGridCoord(10, this->getColWidth(),mev->x(), this->getRowWidth(), mev->y());
        QPoint gridpoint = gridcoord->getGridCoord();

        GridCoords *maincoord = new GridCoords();
        maincoord->setMainCoord(10, this->getColWidth(), gridpoint.x(), this->getRowWidth(), gridpoint.y());
        QPoint mainpoint = maincoord->getMainCoord();

        //qDebug() << "Mouse clicked" << mev->x() << "," << mev->y();
        qDebug() << "Grid: " << gridpoint;
        qDebug() << "Main: " << mainpoint;

        if(mev->buttons() & Qt::LeftButton){
            qDebug() << "left button pressed";
            leftpressed = true;
            rightpressed = true;
            if(startlocation == true)
            {
                startPoint = mainpoint;
                grid_startPoint = gridpoint;
            }
            else if(endlocation == true)
            {
                endPoint = mainpoint;
                grid_endPoint = gridpoint;
            }
            else if((selectobstacle == true))
            {
                if(obstaclecount < 98){
                    obstacles[obstaclecount] = mainpoint;
                    grid_obstacles[obstaclecount] = gridpoint;
                    if(obstaclecount == 0)
                    {
                        obstaclecount++;
                        ui->obstacle_count_label->setText(QString::number(obstaclecount));
                        qDebug() << "Obstacles: " << obstaclecount;
                    }else{
                        qDebug() << "obstacles > 0";
                        for(int i=0; i<obstaclecount; i++)
                        {
                            qDebug() << i << " " << grid_obstacles[i] << " " << grid_obstacles[obstaclecount];
                            if(grid_obstacles[i] == grid_obstacles[obstaclecount])
                            {
                                qDebug() <<i << " Already exists";
                                grid_obstacles[obstaclecount].isNull();
                                i = 100;
                            }
                            if(i == obstaclecount-1){
                                obstaclecount++;
                                ui->obstacle_count_label->setText(QString::number(obstaclecount));
                                qDebug() << "OK LOCATION -> Obstacles: " << obstaclecount;
                                i = 100;
                            }
                        }
                    }
                }
            }
        }
        else if(mev->buttons() & Qt::RightButton)
        {
            qDebug() << "right button pressed";
            rightpressed = true;

            // remove obstacles, start locations and finish locations when pressed
            if((selectobstacle == true))
            {
                if(obstaclecount > 0){
                    //obstacles[obstaclecount] = mainpoint;
                    obstacles[obstaclecount].isNull();
                    obstaclecount--;
                    ui->obstacle_count_label->setText(QString::number(obstaclecount));
                    qDebug() << "Obstacles: " << obstaclecount;
                }
            }
        }

        update();   // updates the painted GUI
    }
}

void MainWindow::send_mqtt_frame(unsigned char frametype, int paylen, unsigned char *pay)
{
    // connect to server
    MQTTClient client;
    MQTTClient_connectOptions opts = MQTTClient_connectOptions_initializer;
    MQTTClient_message pubmsg = MQTTClient_message_initializer;
    MQTTClient_deliveryToken token;
    MQTTClient_create(&client, ADDRESS, CLIENTID, MQTTCLIENT_PERSISTENCE_NONE, NULL);
    opts.keepAliveInterval = 20;
    opts.cleansession = 1;
    opts.username = AUTHMETHOD;
    opts.password = AUTHTOKEN;
    int rc;
    if ((rc = MQTTClient_connect(client, &opts)) != MQTTCLIENT_SUCCESS) {
        qDebug() << "Failed to connect, return code " << rc;
    }
    int framelen = paylen + 5;
    qDebug() << "Frame len: " << framelen;
    unsigned char frame[framelen] = {0};
    unsigned char delimter = 1;
    unsigned char checksum = paylen + delimter;
    unsigned char clientID = 0;
    int frameinc = 0;
    frame[frameinc] = delimter;
    frameinc++;
    frame[frameinc] = clientID;
    frameinc++;
    frame[frameinc] = frametype;
    frameinc++;
    frame[frameinc] = paylen;
    for(int i=0; i<paylen; i++)
    {
        frameinc++;
        frame[frameinc] = pay[i];
        qDebug() << pay[i];
    }
    frameinc++;
    qDebug() << "Frame inc: " << frameinc;
    frame[frameinc] = checksum;
    qDebug() << "Checksum: " << (int)frame[frameinc];

    pubmsg.payload = frame;
    pubmsg.payloadlen = sizeof(frame)/sizeof(frame[0]);
    pubmsg.qos = QOS;
    pubmsg.retained = 0;
    MQTTClient_publishMessage(client, TOPIC, &pubmsg, &token);
    qDebug()<< "Waiting for up to " << (int)(TIMEOUT/1000) <<
               " seconds for publication of " << pay <<
               " \non topic " << TOPIC << " for ClientID: " << CLIENTID;
    rc = MQTTClient_waitForCompletion(client, token, TIMEOUT);
    qDebug() << "Message with token " << (int)token << " delivered.";
    MQTTClient_disconnect(client, 10000);
    MQTTClient_destroy(&client);
}


void MainWindow::on_start_loc_Button_clicked()
{
    qDebug() << "in start location slot";
    if(selectobstacle == false)
    {
        startlocation = true;
    }
    else
    {
        QMessageBox msgBox;
        msgBox.warning(this,"Obstacles not set", "Please finish selecting all obtsacles");
        qDebug() << "QMessageBox: please finish selecting all obtsacles";
    }
}

void MainWindow::on_end_loc_Button_clicked()
{
    qDebug() << "in end location slot";
    if(selectobstacle == false){
        endlocation = true;
    }
    else
    {
        QMessageBox msgBox;
        msgBox.warning(this,"Obstacles not set", "Please finish selecting all obtsacles");
        qDebug() << "QMessageBox: please finish selecting all obtsacles";
    }
}

void MainWindow::on_start_obstacle_Button_clicked()
{
    qDebug() << "in start obstacle selection slot";
    selectobstacle = true;
}

void MainWindow::on_set_obs_Button_clicked()
{
    qDebug() << "in obstacle set slot";
    selectobstacle = false;
    obstaclesSet = true;
}

void MainWindow::on_path_plan_Button_clicked()
{
    if(selectobstacle == false){
        qDebug() << "in path plan slot";
        // check that there is a start location and finish location selected
        qDebug() << "Sending locations to rpi" << grid_startPoint;
        qDebug() << "bool startlocation: " << startlocation;
        if(startlocationSet == true) {
            // need to change to publish frame
            //thread->send_usb_frame('s', grid_startPoint);
            unsigned char pay[] = {(unsigned char)grid_startPoint.x(),(unsigned char)grid_startPoint.y()};
            int pay_len = sizeof(pay)/sizeof(pay[0]);
            send_mqtt_frame('s', pay_len, pay);
        }
        else qDebug() << "No start location set";

        usleep(100000);

        // need to change to publish frame
        if(endlocationSet == true)
        {
            //thread->send_usb_frame('f', grid_endPoint);
            unsigned char pay[] = {(unsigned char)grid_endPoint.x(),(unsigned char)grid_endPoint.y()};
            int pay_len = sizeof(pay)/sizeof(pay[0]);
            send_mqtt_frame('f', pay_len, pay);
        }
        else  qDebug() << "No finish location set";

        usleep(100000);
        if(obstaclesSet == true)
        {
            unsigned char obstacle_arr[100] = {0};
            for(int i=0; i<obstaclecount; i++)
            {
                int index = grid_obstacles[i].x() + grid_obstacles[i].y()*10;
                qDebug() << grid_obstacles[i] << "-> " <<index;
                obstacle_arr[index] = 1;
            }
            int obs_len = sizeof(obstacle_arr)/sizeof(obstacle_arr[0]);

            // No obstacles to start
            send_mqtt_frame('o', obs_len, obstacle_arr);

            //testing code
            //qDebug() << "Starting timer";
//            timer.start();
        }
        else  qDebug() << "No obstacle locations set";
    }
    else
    {
        QMessageBox msgBox;
        msgBox.warning(this,"Obstacles not set", "Please finish selecting all obtsacles");
        qDebug() << "QMessageBox: please finish selecting all obtsacles";
    }


}

void MainWindow::received_frame(unsigned char frame_type, int payload_len, unsigned char *payload)
{
    qDebug() << "Main: Frame type: " << frame_type;
    qDebug() << "Main: Payload Length: " << payload_len;
    //    for(int i=0; i<payload_len; i++)
    //    {
    //        qDebug() << payload[i];
    //    }
    switch(frame_type)
    {
    case 'a':
        qDebug() << "ACK received";
        switch(payload[0])
        {

        case 'f':
            qDebug() << "*** Traverse finished ***";
            ui->message_textbox->clear();
            ui->message_textbox->insertPlainText("Robot has reached its destination");
            break;
        case 'p':
            qDebug() << "Path drawn ACK received by RPI";
            // go to method to begin traversing the robot true the path
            // send robot current location i.e start location
            qDebug() << gridPath[0].x() << "," << gridPath[0].y();
            currentLoc.setX(gridPath[0].x());
            currentLoc.setY(gridPath[0].y());
            unsigned char pay[] = {(unsigned char)gridPath[0].x(),(unsigned char)gridPath[0].y()};
            int pay_len = sizeof(pay)/sizeof(pay[0]);
            send_mqtt_frame('t', pay_len, pay);
            //thread->send_usb_frame('t',gridPath[0]);
            break;

        }
        break;

    case 't':

        if(payload[0] == 'S'){
            qDebug() << "move south: y+1";
            gridTraverse[traverseInc].setX(currentLoc.x());
            gridTraverse[traverseInc].setY(currentLoc.y() + 1);
            unsigned char pay[] = {(unsigned char)gridTraverse[traverseInc].x(),(unsigned char)gridTraverse[traverseInc].y()};
            int pay_len = sizeof(pay)/sizeof(pay[0]);
            send_mqtt_frame('t', pay_len, pay);
            //thread->send_usb_frame('t',gridTraverse[traverseInc]);
            currentLoc = gridTraverse[traverseInc];
        }
        else if(payload[0] == 'N'){
            qDebug() << "move north: y-1";
            gridTraverse[traverseInc].setX(currentLoc.x());
            gridTraverse[traverseInc].setY(currentLoc.y() - 1);
            unsigned char pay[] = {(unsigned char)gridTraverse[traverseInc].x(),(unsigned char)gridTraverse[traverseInc].y()};
            int pay_len = sizeof(pay)/sizeof(pay[0]);
            send_mqtt_frame('t', pay_len, pay);
            //thread->send_usb_frame('t',gridTraverse[traverseInc]);
            currentLoc = gridTraverse[traverseInc];
        }
        else if(payload[0] == 'E'){
            qDebug() << "move east: x+1";
            gridTraverse[traverseInc].setX(currentLoc.x()+1);
            gridTraverse[traverseInc].setY(currentLoc.y());
            unsigned char pay[] = {(unsigned char)gridTraverse[traverseInc].x(),(unsigned char)gridTraverse[traverseInc].y()};
            int pay_len = sizeof(pay)/sizeof(pay[0]);
            send_mqtt_frame('t', pay_len, pay);
            //thread->send_usb_frame('t',gridTraverse[traverseInc]);
            currentLoc = gridTraverse[traverseInc];
        }
        else if(payload[0] == 'W'){
            qDebug() << "move east: x-1";
            gridTraverse[traverseInc].setX(currentLoc.x()-1);
            gridTraverse[traverseInc].setY(currentLoc.y());
            unsigned char pay[] = {(unsigned char)gridTraverse[traverseInc].x(),(unsigned char)gridTraverse[traverseInc].y()};
            int pay_len = sizeof(pay)/sizeof(pay[0]);
            send_mqtt_frame('t', pay_len, pay);
            //thread->send_usb_frame('t',gridTraverse[traverseInc]);
            currentLoc = gridTraverse[traverseInc];
        }
        // set robot locations for main coordinate system
        traverseInc++;
        robotTraversing = true;
        //update();
        break;

    case 'p':
        //testing code
//        testArray[testnum] = timer.elapsed();
//        ui->message_textbox->clear();
//        ui->message_textbox->insertPlainText("Test: " + QString::number(testnum));
//        testnum++;
//        int numbertests = 100;
//        if(testnum == numbertests){
//            testing = false;
//            for(int i=0; i<numbertests;i++){
//                if(i == 0) qDebug() << "Test Number, Time (ms), Path Length";
//                qDebug() << i << ", " << testArray[i] << ", " << payload_len/2;
//            }
//        }else testing = true;
//        // *********************************

        int inc = 0;
        GridCoords *maincoord = new GridCoords();
        for(int i=0; i<payload_len; i++)
        {
            gridPath[inc].setX(payload[i]);
            i++;
            gridPath[inc].setY(payload[i]);
            maincoord->setMainCoord(10, this->getColWidth(), gridPath[inc].x(), this->getRowWidth(), gridPath[inc].y());
            mainPath[inc] = maincoord->getMainCoord();
            qDebug() << "(" << gridPath[inc].x() << "," << gridPath[inc].y() << ")"
                     << " (" << mainPath[inc].x() << "," << mainPath[inc].y() << ")";
            inc++;
        }
        path_points_total = inc;
        qDebug() << "Out of path loop";
        path = true;
        firstpathDraw = true;
        //update();
        break;

    }
    update();
}

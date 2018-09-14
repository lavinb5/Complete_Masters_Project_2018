#ifndef GRIDCOORDS_H
#define GRIDCOORDS_H

#include <QtCore>
#include <QtGui>

class GridCoords
{
private:
    QPoint gridcoord;
    QPoint maincoord;
public:
    GridCoords();
    ~GridCoords();
    void setGridCoord(int offset, int colWidth, int maincoordX, int rowWidth, int maincoordY);
    QPoint getGridCoord();
    void setMainCoord(int offset, int colWidth, int gridcoordX, int rowWidth, int gridcoordY);
    QPoint getMainCoord();
};

#endif // GRIDCOORDS_H

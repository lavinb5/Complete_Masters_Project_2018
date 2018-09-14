#include "gridcoords.h"

GridCoords::GridCoords()
{

}

GridCoords::~GridCoords()
{
    delete this;
    qDebug() << "deleting object";
}

void GridCoords::setGridCoord(int offset, int colWidth, int maincoordX, int rowWidth, int maincoordY)
{
    //gridcoord;
    int x = (maincoordX - offset) / colWidth;
    int y = (maincoordY - offset) / rowWidth;
    gridcoord.setX(x);
    gridcoord.setY(y);
}

QPoint GridCoords::getGridCoord()
{
    return gridcoord;
}

void GridCoords::setMainCoord(int offset, int colWidth, int gridcoordX, int rowWidth, int gridcoordY)
{
    int x = ((colWidth * gridcoordX) + (colWidth/2) + offset);
    int y = ((rowWidth * gridcoordY) + (rowWidth/2) + offset);
    maincoord.setX(x);
    maincoord.setY(y);
}

QPoint GridCoords::getMainCoord()
{
    return maincoord;
}



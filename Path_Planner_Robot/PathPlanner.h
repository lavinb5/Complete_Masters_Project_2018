/*
 * PathPlanner.h
 *
 *  Created on: 26 Jun 2018
 *      Author: bryan
 */

#ifndef PATHPLANNER_H_
#define PATHPLANNER_H_

#include "Writer.h"

class PathPlanner {

private:
	int ROW, COL;
	unsigned char startlocation[2];
	unsigned char finishlocation[2];
	unsigned char obstacles[100], mat_obs[10][10];
	bool closedlist[10][10];
	bool startSet, finishSet, obstaclesSet, pathSet;



	struct openstr{
		int points[2];
		double f;
	};
	int openlist_inc;
	bool destFound;

public:
	struct cell{
			int parent[2];
			double f,g,h;
	};
	struct path{
			int points[2];
	};
	path plannedPath[100];
	int plannedPathSize;
	PathPlanner();
	virtual ~PathPlanner();
	void received_frame(unsigned char, int, unsigned char*);
	void display_setlocations();
	void aStarSearchTest();
	bool isValid(int col, int row);
	bool isBlocked(int col, int row);
	bool isDestination(int col, int row);
	void initialCheck();
	double manhattan(int, int);
	void tracePath(cell[][10]);
	void sendPath(int, path*);
	char direction(int,int,int,int);
	Writer wr;
};

#endif /* PATHPLANNER_H_ */

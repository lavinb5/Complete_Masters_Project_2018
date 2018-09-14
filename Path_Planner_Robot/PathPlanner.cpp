/*
 * PathPlanner.cpp
 *
 *  Created on: 26 Jun 2018
 *      Author: bryan
 */

#include "PathPlanner.h"
#include <float.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <iostream>
using namespace std;

PathPlanner::PathPlanner() {
	// TODO Auto-generated constructor stub
	openlist_inc = 0;
	destFound = false;
	startSet = false;
	finishSet = false;
	obstaclesSet = false;
	pathSet = false;
	ROW = 10;
	COL = 10;
	plannedPathSize = 0;
}

PathPlanner::~PathPlanner() {
	// TODO Auto-generated destructor stub
}

void PathPlanner::received_frame(unsigned char frametype, int pay_len,
		unsigned char *pay) {
	//cout << "In reveived method" << endl;
	int ind=0;
	switch(frametype)
	{
	case 'o':
		cout << "Obstcle: payload lebgth -> " << pay_len << endl;
		obstaclesSet = true;
		for(int i=0; i<pay_len; i++)
		{
			obstacles[i] = pay[i];
		}
		//this->display_setlocations();
		//cout << "Obstacles Map" << endl;
		for(int row=0; row<10;row++)
		{
			for(int col=0; col<10; col++)
			{
				mat_obs[col][row] = obstacles[ind];
				ind++;
				//cout << (int)mat_obs[col][row] << " ";
			}
			cout << endl;
		}
		pathSet = false;
		break;

	case 's':
		cout << "Start Location: payload lebgth -> " << pay_len << endl;
		startSet = true;
		for(int i=0; i<pay_len; i++)
		{
			startlocation[i] = pay[i];
		}
		pathSet = false;
		// send acknowledge that start loc is set
		//this->display_setlocations();
		break;

	case 'f':
		cout << "Finish Location: payload lebgth -> " << pay_len << endl;
		finishSet = true;
		for(int i=0; i<pay_len; i++)
		{
			finishlocation[i] = pay[i];
		}
		//this->display_setlocations();
		pathSet = false;
		break;

	case 'c':
		cout << "Current Location: payload lebgth -> " << pay_len << endl;
		break;

	case 'p':
		cout << "Plan path: payload lebgth -> " << pay_len << endl;
		break;

		// ACK from GUI
	case 'a':
		cout << "ACK received from GUI" << endl;
		cout << "ACK type: " << (char)pay[0] << endl;
		switch(pay[0])
		{
		case 'p':
			cout << "Path Drawn by GUI" << endl;
			unsigned char pay[] = {'p'};
			// write frame
			wr.write_frame('a', 1, pay);
			break;
		}
		break;

		case 't':
			cout << "Traverse frame received" << endl;
			cout << "received location: " << (int)pay[0] << ", " << (int)pay[1] << endl;

			//******* COOPERATIVE******
			//send current location to other robot in network
			unsigned char current[3];
			current[0] = 'c';
			current[1] = pay[0];
			current[2] = pay[1];
			wr.write_frame('a', 3, current);
			//**************************************************

			cout << "Path size" << plannedPathSize << endl;
			for(int i =0; i<plannedPathSize+1; i++){
				//cout << "i: " << i << "   ";
				//cout << plannedPath[i].points[0] << ", " << plannedPath[i].points[1] << endl;
				if(((int)pay[0] == plannedPath[i].points[0])&&((int)pay[1] == plannedPath[i].points[1]))
					//if(pay[0] == plannedPath[i].points[0])
				{
					if((int)pay[0] == finishlocation[0] && (int)pay[1] == finishlocation[1])
					{
						// robot has reached destination
						cout << "*** Robot has reached destination ***" << endl;
						unsigned char pay[] = {'f'};
						wr.write_frame('a', 1, pay);
						i = plannedPathSize + 1;
					}else
					{
						int ind = i;
						cout << "Match at index: " << ind << endl;
						// calculate direction
						char moveDirection= direction(plannedPath[i].points[0], plannedPath[i].points[1], plannedPath[i-1].points[0], plannedPath[i-1].points[1]);
						cout << moveDirection << endl;
						unsigned char pay[] = {moveDirection};
						wr.write_frame('t', 1, pay);
						i = plannedPathSize + 1;
					}
				}
			}
			break;

	}

	if((pathSet == false && startSet == true && finishSet == true && obstaclesSet == true))
	{
		cout << "call A* func" << endl;
		this->aStarSearchTest();
		startSet = false;
		finishSet = false;
		obstaclesSet = false;
	}

}

void PathPlanner::display_setlocations() {
	cout << "Path Planner: Display method" << endl;
	if(startSet == true)
	{
		cout << "Start location: (" << (int)startlocation[0] << ", " << (int)startlocation[1] << ")" << endl;
	}
	else cout << "Start location not received" << endl;
	if(finishSet == true)
	{
		cout << "Finish location: (" << (int)finishlocation[0] << ", " << (int)finishlocation[1] << ")" << endl;
	}
	else cout << "Finish location not received" << endl;
	if(obstaclesSet == true)
	{
		for(int row=0; row<ROW;row++)
		{
			for(int col=0; col<COL; col++)
			{
				cout << (int)mat_obs[col][row] << " ";
			}
			cout << endl;
		}
	}
	else cout << "Obstacles not received" << endl;
}

void PathPlanner::tracePath(cell cellDetails[][10]) {
	cout << "The optimal path is: " << endl;
	int x = finishlocation[0];
	int y = finishlocation[1];

	//path plannedPath[100];
	int path_inc = 0;
	while(!(cellDetails[x][y].parent[0] == x && cellDetails[x][y].parent[1] == y))
	{
		plannedPath[path_inc].points[0] = x;
		plannedPath[path_inc].points[1] = y;
		int temp_x = cellDetails[x][y].parent[0];
		int temp_y = cellDetails[x][y].parent[1];
		x = temp_x;
		y = temp_y;
		path_inc++;
	}
	plannedPathSize = path_inc + 1; // includes start point
	for(int i=0; i<path_inc;i++)
	{
		cout << "(" << plannedPath[i].points[0] << "," << plannedPath[i].points[1] << ") -> ";
	}
	pathSet = true;

	this->sendPath(path_inc, plannedPath);

}

void PathPlanner::aStarSearchTest() {
	// display locations and obstacle map
	this->display_setlocations();

	// check initial conditions before starting algorithm
	this->initialCheck();
	cout << "Initial check ok!" << endl;

	// set closed list to false -> no members on list yet
	for(int row=0; row<ROW;row++)
	{
		for(int col=0; col<COL; col++)
		{
			closedlist[col][row] = false;
		}
	}

	// array of each cell and its parameters
	//cell cellDetails[COL][ROW];
	cell cellDetails[10][10];

	// x and y cooridinate of the grid
	int x,y;

	// set cell details to values that should not exist on the grid
	for(y=0; y<ROW; y++)
	{
		for(x=0; x<COL; x++)
		{
			cellDetails[x][y].f = FLT_MAX;
			cellDetails[x][y].g = FLT_MAX;
			cellDetails[x][y].h = FLT_MAX;
			cellDetails[x][y].parent[0] = -1;	// grid cant be (-1,-1);
			cellDetails[x][y].parent[1] = -1;
		}
	}


	// Initialise parameters of starting node
	x = startlocation[0], y = startlocation[1];
	cellDetails[x][y].f = 0.0;
	cellDetails[x][y].g = 0.0;
	cellDetails[x][y].h = 0.0;
	cellDetails[x][y].parent[0] = x;
	cellDetails[x][y].parent[1] = y;



	// create open list
	int max_nodes = 100;
	openstr openlist[max_nodes];
	openlist_inc=0;
	for(int i=0; i<max_nodes; i++)
	{
		openlist[i].f = 0.0;
		openlist[i].points[0] = -1;
		openlist[i].points[1] = -1;
	}

	openlist[openlist_inc].f = 0.0;
	openlist[openlist_inc].points[0] = x;
	openlist[openlist_inc].points[1] = y;

	// set destination found flag to false;
	destFound = false;

	cout << "Entering while loop" << endl;

	//x = openlist[openlist_inc].points[0];
	//y = openlist[openlist_inc].points[1];
	closedlist[x][y] = true;

	bool firstcell_complete = false;
	// begin to search the grid
	while(destFound == false)
	{
		//sleep(1);

		if(firstcell_complete == true)
		{
			double fmin = FLT_MAX;
			int current_x = x, current_y = y;
			if(openlist_inc == 0){
				cout << "back to parent cell" << endl;
				x  = cellDetails[current_x][current_y].parent[0];
				y  = cellDetails[current_x][current_y].parent[1];
			}else{
				// find the min f in the openlist
				for(int i=1; i<openlist_inc+1; i++){
					cout << "Fmin open list[" << i << "] = " << openlist[i].f ;
					cout << " Y: " << openlist[i].points[1] << endl;
					if(openlist[i].f < fmin) fmin = openlist[i].f;
				}
				cout << "f(n) min = " << fmin<< endl;

				// set the cell with min f in closed list
				for(int i=1; i<openlist_inc+1; i++){
					if(openlist[i].f == fmin){
						cout<< "Fmin open list[" << i << "] added to open list" << endl;
						closedlist[openlist[i].points[0]][openlist[i].points[1]] = true;
						x = openlist[i].points[0];
						y = openlist[i].points[1];
						cout << "Cell (" << x << ", " << y << ") added";
						cout << " Parents: (" << cellDetails[x][y].parent[0] << "," << cellDetails[x][y].parent[1] << ")" << endl;
					}
				}
				openlist_inc = 0;
			}

		}

		// create new f,g,h for successor nodes
		double fNew, gNew, hNew;


		cout << "x: " << x << "   y: " << y << "   Open list inc: " << openlist_inc << endl;

		// NORTH------------------------NORTH
		if(this->isValid(x,y-1) == true)
		{
			cout << "North valid cell" << endl;
			if(isDestination(x,y-1) == true)
			{
				cellDetails[x][y-1].parent[0] = x;
				cellDetails[x][y-1].parent[1] = y;
				cout << "Destination cell found" << endl;
				// got to draw method
				this->tracePath(cellDetails);
				//NEED TO FIGURE OUT
				cout << "finished trace path" << endl;

				destFound = true;
				return;
			}
			else if((closedlist[x][y-1] == false) && (this->isBlocked(x,y-1) == false))
			{
				//cout << "2. in else if statement" << endl;
				gNew = cellDetails[x][y].g + 1.0;
				hNew = manhattan(x, y-1);
				fNew = gNew + hNew;
				cout << "North: g(n) = " << gNew << " h(n) = " << hNew << " f(n) = " << fNew << endl;
				if(cellDetails[x][y-1].f == FLT_MAX || cellDetails[x][y-1].f >= fNew)
				{
					//cout << "North: Adding to openlist" << endl;
					openlist_inc++;
					openlist[openlist_inc].f = fNew;
					openlist[openlist_inc].points[0] = x;
					openlist[openlist_inc].points[1] = y-1;
					//cout << "North: Adding to openlist -- ";
					//cout << openlist[openlist_inc].points[0] << ", " << openlist[openlist_inc].points[1];
					//cout << " index: " << openlist_inc;
					//cout << " f(n): " << openlist[openlist_inc].f << endl;;

					cellDetails[x][y-1].f = fNew;
					cellDetails[x][y-1].g = gNew;
					cellDetails[x][y-1].h = hNew;
					cellDetails[x][y-1].parent[0] = x;
					cellDetails[x][y-1].parent[1] = y;
				}else cout << "North: can't add to openlist" << endl;
			}else cout << "north closed list" << closedlist[x][y-1] << endl;
			//cout << "4. out" <<endl;

		}else cout << "North not vaild cell" << endl;



		//cout << "x: " << x << "y: " << y << endl;


		// SOUTH------------------------SOUTH
		if(this->isValid(x,y+1) == true)
		{
			cout << "South valid cell" << endl;
			if(isDestination(x,y+1) == true)
			{
				cellDetails[x][y+1].parent[0] = x;
				cellDetails[x][y+1].parent[1] = y;
				cout << "Destination cell found" << endl;
				// got to draw method
				this->tracePath(cellDetails);
				destFound = true;
				return;
			}
			else if((closedlist[x][y+1] == false) && (this->isBlocked(x,y+1) == false))
			{
				gNew = cellDetails[x][y].g + 1.0;
				hNew = manhattan(x, y+1);
				fNew = gNew + hNew;
				cout << "South: g(n) = " << gNew << " h(n) = " << hNew << " f(n) = " << fNew << endl;
				if(cellDetails[x][y+1].f == FLT_MAX || cellDetails[x][y+1].f >= fNew)
				{
					cout << "South: Adding to openlist" << endl;
					openlist_inc++;
					openlist[openlist_inc].f = fNew;
					openlist[openlist_inc].points[0] = x;
					openlist[openlist_inc].points[1] = y+1;

					cellDetails[x][y+1].f = fNew;
					cellDetails[x][y+1].g = gNew;
					cellDetails[x][y+1].h = hNew;
					cellDetails[x][y+1].parent[0] = x;
					cellDetails[x][y+1].parent[1] = y;
				}else cout << "South: can't add to openlist" << endl;
			}
		}else cout << "South not vaild cell" << endl;




		// EAST--------------------------EAST
		if(this->isValid(x+1,y) == true)
		{
			if(isDestination(x+1,y) == true)
			{
				cellDetails[x+1][y].parent[0] = x;
				cellDetails[x+1][y].parent[1] = y;
				cout << "Destination cell found" << endl;
				// go to draw method
				this->tracePath(cellDetails);
				destFound = true;
				return;
			}
			else if((closedlist[x+1][y] == false) && (this->isBlocked(x+1,y) == false))
			{
				gNew = cellDetails[x][y].g + 1.0;
				hNew = manhattan(x+1,y);
				fNew = gNew + hNew;
				cout << "East: g(n) = " << gNew << " h(n) = " << hNew << " f(n) = " << fNew << endl;
				if(cellDetails[x+1][y].f == FLT_MAX || cellDetails[x+1][y].f >= fNew)
				{
					cout << "East: Adding to openlist" << endl;
					openlist_inc++;
					openlist[openlist_inc].f = fNew;
					openlist[openlist_inc].points[0] = x+1;
					openlist[openlist_inc].points[1] = y;

					cellDetails[x+1][y].f = fNew;
					cellDetails[x+1][y].g = gNew;
					cellDetails[x+1][y].h = hNew;
					cellDetails[x+1][y].parent[0] = x;
					cellDetails[x+1][y].parent[1] = y;
				}
			}
		}else cout << "East not vaild cell" << endl;

		//cout << "x: " << x << "y: " << y << endl;
		// WEST--------------------------WEST
		if(this->isValid(x-1,y) == true)
		{
			if(isDestination(x-1,y) == true)
			{
				cellDetails[x-1][y].parent[0] = x;
				cellDetails[x-1][y].parent[1] = y;
				cout << "Destination cell found" << endl;
				// got to draw method
				this->tracePath(cellDetails);
				destFound = true;
				return;
			}
			else if((closedlist[x-1][y] == false) && (this->isBlocked(x-1,y) == false))
			{
				gNew = cellDetails[x][y].g + 1.0;
				hNew = manhattan(x-1,y);
				fNew = gNew + hNew;
				cout << "West: g(n) = " << gNew << " h(n) = " << hNew << " f(n) = " << fNew << endl;
				if(cellDetails[x-1][y].f == FLT_MAX || cellDetails[x-1][y].f >= fNew)
				{
					cout << "West: Adding to openlist" << endl;
					openlist_inc++;
					openlist[openlist_inc].f = fNew;
					openlist[openlist_inc].points[0] = x-1;
					openlist[openlist_inc].points[1] = y;

					cellDetails[x-1][y].f = fNew;
					cellDetails[x-1][y].g = gNew;
					cellDetails[x-1][y].h = hNew;
					cellDetails[x-1][y].parent[0] = x;
					cellDetails[x-1][y].parent[1] = y;
				}
			}
		}else cout << "West not vaild cell" << endl;

		firstcell_complete = true;
	}
	if(destFound == false)
	{
		cout << "Failed to find a path" << endl;
	}

}

bool PathPlanner::isValid(int col, int row) {
	return (col >=0) && (col < COL) && (row >=0) && (row < ROW);
}

bool PathPlanner::isBlocked(int col, int row) {
	if(mat_obs[col][row] == 1) return true;
	else return false;
}

bool PathPlanner::isDestination(int col, int row) {
	if(col == finishlocation[0] && row == finishlocation[1]) return true;
	else return false;
}

void PathPlanner::initialCheck() {
	// check that the start location is not out of range
	if(this->isValid(startlocation[0], startlocation[1]) == false)
	{
		cout << "Error: The Start location is out of range" << endl;
		return;
	}
	// check that the finish location is not out of range
	if(this->isValid(finishlocation[0], finishlocation[1]) == false)
	{
		cout << "Error: The Finish location is out of range" << endl;
		return;
	}
	// check if either the start or finish locations are blocked
	if(isBlocked(startlocation[0], startlocation[1]) == true || isBlocked(finishlocation[0], finishlocation[1]) == true)
	{
		cout << "Error: The Start or Finish location are blocked" << endl;
		return;
	}
	// check if the start location is the same as finish location
	if(isDestination(startlocation[0], startlocation[1]) == true)
	{
		cout << "Error: Start location same as finish" << endl;
		return;
	}
}

double PathPlanner::manhattan(int x, int y) {
	double dx1, dy1, dx2, dy2;
	dx1 = abs(x - finishlocation[0]);
	dy1 = abs(y - finishlocation[1]);
	double h = dx1+dy1;
	//double h = sqrt((dx1*dx1)+(dy1*dy1));
	dx2 = abs(startlocation[0] - finishlocation[0]);
	dy2 = abs(startlocation[1] - finishlocation[1]);
	double cross = (abs((dx1*dy2)-(dx2*dy1)))*0.01;
	return h + cross;
	//return h;
}

void PathPlanner::sendPath(int pathsize, path *p) {
	cout << "Sending path to GUI" << endl;
	unsigned char path_arr[255] = {0}, arr_len;
	for(int i=pathsize; i>=0; i--)
	{
		if(i == pathsize){
			if(p[i].points[0] != startlocation[0] || p[i].points[1] != startlocation[1])
			{
				p[i].points[0]= startlocation[0];
				p[i].points[1]= startlocation[1];
			}
		}
		cout << "["<< i <<"]: (" << p[i].points[0] << "," << p[i].points[1] << ")" << endl;
	}
	arr_len = (pathsize*2) + 2;
	int a = 0;
	for(int i=pathsize; i>=0; i--)
	{
		cout << "[" << i << "]" << endl;
		for(int x=0; x<2; x++)
		{
			if(x == 0)path_arr[a] = p[i].points[x];
			else if(x == 1){
				a += 1;
				path_arr[a] = p[i].points[x];
			}
		}
		a += 1;
	}
	cout << "a: " << a << endl;
	cout << "length of path array: " << (int)arr_len << endl;
	for(int i=0; i<(int)arr_len; i++)
	{
		cout << (int)path_arr[i] << " ";
	}
	cout << endl;

	wr.write_frame('p', (int)arr_len, path_arr);
}

char PathPlanner::direction(int curx, int cury, int nextx, int nexty) {
	int x,y;
	x = nextx-curx;
	y = nexty-cury;
	cout << x << ", " << y << endl;
	if(x==0 && y==-1) return 'N';
	else if(x==0 && y==1) return 'S';
	else if(x==1 && y==0) return 'E';
	else if(x==-1 && y==0) return 'W';
	return 'X';
}

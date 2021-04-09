//
// Created by Zhang Pengfei on 10/1/2017.
//

#ifndef PHREAD_UTIL_H
#define PHREAD_UTIL_H

#endif //PHREAD_UTIL_H
#ifndef __UTIL_H_
#define __UTIL_H_

#define SIZE 1000

/* data struct of Point */

typedef struct point{
    int ID;
    float * values;
} Point;

void printPoint(Point a,int dim);

void printPoints(Point*, int,int);

void writePointsToFile(Point* points, char* destFilename, int numPt, int dim);

void writePermissiblePtToFile(Point* pts, int numPt, char* fileName);

Point* generateDataset(int number, int dim, float arg, int seed);

int readPoints(char* fileName, Point** subPts, int dim, int offset, int limit);

#endif

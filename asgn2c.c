#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include <mpi.h>
#include <assert.h>
#include<pthread.h>
#include"util.h"

// If you have referenced to any source code that is not written by you
// You have to cite them here.

int main(int agrc, char* argv[]) {
  if(agrc != 5){
    printf("Usage: %s <Number of point> <dimension> <Path of input dataset> <Path of output result>\n", argv[0]);
    return 1;
  }
  int numPt = atoi(argv[1]);        //number of points in dataset
  int dim = atoi(argv[2]);          //the dimension of the dataset
  char* pathDataset = argv[3];      //the path of dataset
  char* pathOutput = argv[4];       //the path of your file


  /**********************************************************************************
   * Work here, you need to modify the code below
   * Here we just demonstrate some usage of helper function
   * *******************************************************************************/
  Point* pts;
  int offset = 0;                   //skip that many points before beginning to return points in readPoints()
  int limit = numPt;                //number of points to return in readPoints()

  readPoints(pathDataset, &pts, dim, offset, limit);  //Read points from file 'pathDataset' and store in 'pts'

  int permissiblePointNum = 0;
  Point* pPermissiblePoints = NULL;

  writePermissiblePtToFile(pPermissiblePoints, permissiblePointNum, pathOutput); //write the result to file 'pathOutput'

  return 0;
}
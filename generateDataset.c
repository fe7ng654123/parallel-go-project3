//
// Created by Chris Liu on 8/10/2019.
//
#include <stdlib.h>
#include <stdio.h>
#include "util.h"

int main(int agrc, char* argv[]) {

  if(agrc != 6){
    printf("Usage: %s {Numver of point} {dimension} {arg} {seed} {destination file name}", argv[0]);
    return 1;
  }
  int number = atoi(argv[1]);
  int dim = atoi(argv[2]);
  float arg = (float) atof(argv[3]);
  int seed = atoi(argv[4]);
  char* dstFileName = argv[5];

  Point* datasetPts = generateDataset(number, dim, arg, seed);
  writePointsToFile(datasetPts, dstFileName, number, dim);
  printf("Successfully generated dataset to %s\n", dstFileName);
}
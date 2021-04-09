//
// Created by Zhang Pengfei on 10/1/2017.
//

#include <stdio.h>
#include <stdlib.h>
#include "util.h"
#include "WjCryptLib/WjCryptLib_Rc4.h"


static float* generate_indep(int dim);
static float* generate_anti(int dim, float arg);
static float random_normal(float med, float var) ;
static float random_peak(float min, float max, int dim);
static float random_equal(float min, float max);
static int is_vector_ok(int dim, float *x);
extern int iter;

Rc4Context rc4_context;

Point* generateDataset(int number, int dim, float arg, int seed) {

    Point * points = (Point*)malloc(sizeof(Point) * number);
	if(points == NULL)
	{
		printf("Cannot allocate memory for points.\n");
		return NULL;
	}
    int i, j;
    Rc4Initialise(&rc4_context, &seed, sizeof(seed), 512);
    printf("\nGenerating %d-dimension point dataset, number of points in dataset = %d\n", dim, number);
	for(i = 0; i < number; i++) {
		points[i].ID = i + 1;
		points[i].values = generate_anti(dim, arg);
		if(points[i].values == NULL)
			return NULL;
	}
    
    return points;
}

static float* generate_anti(int dim, float arg) {
    float *x = (float*) malloc(sizeof(float) * dim);
	if(x == NULL)
	{
		printf("Cannot allocate memory for values of Point\n");
		return NULL;
	}
    do{
        int d;
        float v = random_normal(0.5, arg);
        float l = v <= 0.5 ? v : 1.0 - v;

        for(d = 0; d < dim; d++)
            x[d] = v;
        for(d = 0; d < dim; d++){
            float h = random_equal(-l, l);
            x[d] += h;
            x[(d + 1) % dim] -= h;
        }
    } while(!is_vector_ok(dim, x));

    return x;
}

static float random_normal(float med, float var) {
    return random_peak(med - var, med+var, 12);
}

static float random_peak(float min, float max, int dim) {
    int d;
    float sum = 0.0;

    for(d = 0; d < dim; d++) {
        sum += random_equal(0,1);
    }

    sum /= dim;
    return sum * (max - min) + min;
}

static int is_vector_ok(int dim, float *x){
    while(dim--){
        if(*x < 0.0 || *x > 1.0)
            return 0;
        x++;
    }

    return 1;
}


static int rc4_rand() {
    int ret;
    Rc4Output(&rc4_context, &ret, sizeof(ret));
    if (ret < 0) {
        return -ret;
    }
    return ret;
}

static float random_equal(float min, float max){
    float x = (float) rc4_rand() / RAND_MAX;
    return x * (max - min) + min;
}


void printPoint(Point a,int dim){
    printf("%d:", a.ID);
    int i;
    for(i = 0; i < dim-1; i++) {
        printf("%f,", a.values[i]);
        fflush(stdout);
    }
    printf("%f\n", a.values[dim-1]);
}

void printPoints(Point* pts, int number, int dim){
  int counter = 0;
  for(;counter<number;counter++){
    printPoint(pts[counter], dim);
  }
}

void writePointsToFile(Point* points, char* destFilename, int numPt, int dim){
  FILE *fp;
  int x;

  fp = fopen(destFilename, "wb");

  for(x=0; x<numPt; x++){
    fwrite(points[x].values, sizeof(float), dim, fp);
  }
  fclose(fp);
}

int* getIdFromPoints(Point* pts, int numPt){
  int* ret = malloc(sizeof(int)*numPt);

  for(int x = 0; x < numPt; x++){
    ret[x] = pts[x].ID;
  }

  return ret;
}

int int_cmp(const void *a, const void *b)
{
  const int *ia = (const int *)a; // casting pointer types
  const int *ib = (const int *)b;
  return *ia  - *ib;
  /* integer comparison: returns negative if b > a
  and positive if a > b */
}

void writePermissiblePtToFile(Point* pts, int numPt, char* fileName){
  int* ids = getIdFromPoints(pts, numPt);

//  qsort(ids, numPt, sizeof(int), int_cmp);

  FILE *fd = fopen(fileName, "wb");
  fprintf(fd, "%d\n", numPt);
  for(int x = 0; x < numPt; x++){
    fprintf(fd, "%d\n", ids[x]);
  }
  fclose(fd);
}

int calOffset(int offset, int dim){
  return offset * dim * sizeof(float);
}

int readPoints(char* fileName, Point** subPts, int dim, int offset, int limit){
  // Create file description
  // fseek
  // util convert
  FILE * fp;
  int offsetByte = calOffset(offset, dim);
  int counter = 1;

//  getOffsetAndLimit(&offsetByte, &offsetID, &numTuplePerProcess, number, dim, numProcess, myID);

  *subPts = malloc(sizeof(Point)*limit);

  fp = fopen(fileName, "r");
  if (fp == NULL)
  {
    perror("Error while opening the file.\n");
    exit(1);
  }

  fseek(fp, offsetByte, SEEK_SET);

  for(counter=1; counter <= limit; counter++){
    (*subPts)[counter-1].ID = offset + counter;
    (*subPts)[counter-1].values = malloc(sizeof(float)*dim);
    fread((*subPts)[counter-1].values, sizeof(float), dim, fp);
  }
  fclose(fp);
  return limit;
}


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <mpi.h>
#include <assert.h>
#include <pthread.h>
#include <string.h>
#include <stddef.h>
#include "util.h"

// If you have referenced to any source code that is not written by you
// You have to cite them here.

typedef struct point_max_dim
{
  int ID;
  float values[5];
} Point_Max;


int main(int agrc, char *argv[])
{
  if (agrc != 5)
  {
    printf("Usage: %s <Number of point> <dimension> <Path of input dataset> <Path of output result>\n", argv[0]);
    return 1;
  }
  int numPt = atoi(argv[1]);   //number of points in dataset
  int dim = atoi(argv[2]);     //the dimension of the dataset
  char *pathDataset = argv[3]; //the path of dataset
  char *pathOutput = argv[4];  //the path of your file

  /**********************************************************************************
   * Work here, you need to modify the code below
   * Here we just demonstrate some usage of helper function
   * *******************************************************************************/
  MPI_Init(NULL, NULL);

  // Get the number of processes
  int world_size;
  MPI_Comm_size(MPI_COMM_WORLD, &world_size);

  // Get the rank of the process
  int world_rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

  int permissiblePointNum = 0;
  Point *pPermissiblePoints = NULL;

  // pPermissiblePoints = malloc((numPt + 1) / 3 * sizeof(Point));
  // pPermissiblePoints = realloc(pPermissiblePoints, (numPt+1)/3*sizeof(Point));

  // MPI_Aint displacements[2]  = {offsetof(Point_Max, ID), offsetof(Point_Max, values)};
  // int block_lengths[2]  = {1,5};
  // MPI_Datatype types[2] = {MPI_INT, MPI_FLOAT};
  // MPI_Datatype custom_dt;

  // // 2- Create the type, and commit it
  // MPI_Type_create_struct(2, block_lengths, displacements, types, &custom_dt);
  // MPI_Type_commit(&custom_dt);

  Point *pts;
  Point *pts_2;
  int offset = 0;    //skip that many points before beginning to return points in readPoints()
  int limit = numPt; //number of points to return in readPoints()

  // readPoints(pathDataset, &pts, dim, offset, limit);  //Read points from file 'pathDataset' and store in 'pts'

  if (world_rank == 0)
  {
    offset = 0;
    limit = (numPt + 1) / 3;
  }
  else if (world_rank == 1)
  {
    offset = (numPt + 1) / 3;
    limit = (numPt + 1) / 3;
  }
  else
  {
    offset = (numPt + 1) / 3 * 2;
    limit = numPt - ((numPt + 1) / 3 * 2);
  }

  // printf("Rank %d: offset = %d, limit= %d\n", world_rank, offset, limit);
  int split_size = limit;

  int block_size = 250;
  if (split_size <= 250)
    block_size = split_size;

  // printf("split_size = %d\n",split_size);

  int *bitmap = malloc(sizeof(int) * split_size);
  memset(bitmap, 0, sizeof(int) * split_size);

  // int round_no = number / 250 / 3;
  // if (number <= 250)
  //   round_no = 1;
  // printf("rank %d : round_no = %d", world_rank, round_no);

  readPoints(pathDataset, &pts, dim, offset, split_size);
  int round_size = 0;

  int token = numPt;
  offset =0;
  while(token > 0)
  {
    // if (i == 0)
    // {
    //   offset = 0;
    //   round_size = (numPt + 1) / 3;
    //   readPoints(pathDataset, &pts_2, dim, offset, round_size); //first round
    // }
    // else if (i == 1)
    // {
    //   offset = (numPt + 1) / 3;
    //   round_size = (numPt + 1) / 3;
    //   readPoints(pathDataset, &pts_2, dim, offset, round_size); //second round
    // }
    // else if (i == 2)
    // {
    //   offset = (numPt + 1) / 3 * 2;
    //   round_size = numPt - ((numPt + 1) / 3 * 2);
    //   readPoints(pathDataset, &pts_2, dim, offset, round_size); //third round
    // }
    block_size = 250;
    if (token >block_size)
    {
      round_size =block_size;
      token -=block_size;
    }else{
      round_size = token;
      token =0;
    }
    readPoints(pathDataset, &pts_2, dim, offset, round_size); 
    offset+=block_size;
    // for (int i = 0; i < split_size; i+=block_size)
    // {
    //   /* code */
    // }
    
    for (int h = 0; h < split_size; h++)
    {
      if (bitmap[h])
        continue;
      for (int k = 0; k < round_size; k++)
      {
        if (pts[h].ID == pts_2[k].ID)
          continue;
        int counter = 0;
        for (int z = 0; z < dim; z++)
        {
          if (pts[h].values[z] >= pts_2[k].values[z])
          {
            counter++;
          }
          else
            break;
        }
        if (counter == dim)
        {
          // permissible=0;
          bitmap[h] = 1;
          // printf("bitmap[%d] set to 1\n",h);
          break;
        }
      }
    }
    for (int i = 0; i < round_size; i++)
    {
      free(pts_2[i].values);
    }
    
    free(pts_2);
  }
  // for (int i = 0; i < split_size; i++)
  // {
  //   printf("bitmap[%d] = %d\n", i, bitmap[i]);
  //   // if (bitmap[i] != 1)
  //   // {
  //   //   memcpy(&p_result[(*p_num)++], &pts[i], sizeof(Point));
  //   // }
  // }
  free(pts);
  // printf("freed!");


  int *gathered_bitmap = NULL;
  if (world_rank == 0)
  {
    gathered_bitmap = malloc(numPt * sizeof(int));
    // printf("allocated !!!!!");
  }

  int counts[3] = {(numPt + 1) / 3, (numPt + 1) / 3, numPt - ((numPt + 1) / 3 * 2)};
  int disps[3] = {0, counts[0], counts[0] + counts[1]};
  
  MPI_Gatherv(bitmap, split_size, MPI_INT,
              gathered_bitmap, counts, disps, MPI_INT, 0, MPI_COMM_WORLD);
  free(bitmap);
   
  if (world_rank ==0)
  {
    int total_count = counts[0]+counts[1]+counts[2];
    pPermissiblePoints = malloc(total_count*sizeof(Point)); 
    for (int i = 0; i < numPt ; i++)
    {
      // printf("gathered_bitmap[%d] = %d, ID= %d\n", i, gathered_bitmap[i], i+1);
      if (gathered_bitmap[i] != 1)
      {
        pPermissiblePoints[permissiblePointNum++].ID = i+1;
      }
    }
    printf("final permissiblePointNum = %d\n",permissiblePointNum);
    // for (int i = 0; i < permissiblePointNum; i++)
    // {
    //   printf("ID = %d\n",pPermissiblePoints[i].ID);
    // }
    
    writePermissiblePtToFile(pPermissiblePoints, permissiblePointNum, pathOutput);
    free(pPermissiblePoints);
    free(gathered_bitmap);
  }
  
  

  MPI_Finalize();
  return 0;
}
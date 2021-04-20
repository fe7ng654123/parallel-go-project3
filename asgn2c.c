#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include <mpi.h>
#include <assert.h>
#include<pthread.h>
#include<string.h>
#include<stddef.h>
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
  MPI_Init(NULL, NULL);

  // Get the number of processes
  int world_size;
  MPI_Comm_size(MPI_COMM_WORLD, &world_size);

  // Get the rank of the process
  int world_rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

  // printf("Hello world from  rank %d out of %d processors\n", world_rank,world_size);
  int permissiblePointNum = 0;
  Point* pPermissiblePoints = NULL;

  pPermissiblePoints = realloc(pPermissiblePoints, numPt*sizeof(Point));

  MPI_Aint displacements[2]  = {offsetof(Point, ID), offsetof(Point, values)};
  int block_lengths[2]  = {1,dim};
  MPI_Datatype types[2] = {MPI_INT, MPI_FLOAT};
  MPI_Datatype custom_dt;

  // 2- Create the type, and commit it
  MPI_Type_create_struct(2, block_lengths, displacements, types, &custom_dt);
  MPI_Type_commit(&custom_dt);

  Point* pts;
  int offset = 0;                   //skip that many points before beginning to return points in readPoints()
  int limit = numPt;                //number of points to return in readPoints()

  readPoints(pathDataset, &pts, dim, offset, limit);  //Read points from file 'pathDataset' and store in 'pts'

  // if(world_rank == 0){
  //   offset = 0;
  //   limit= numPt/3;
  //   printf("Rank %d: offset = %d, limit= %d\n", world_rank, offset, limit);
  //   readPoints(pathDataset, &pts, dim, offset, limit);
  // }
  // else if (world_rank ==1){
  //   offset = numPt/3;
  //   limit = numPt/3;
  //   printf("Rank %d: offset = %d, limit= %d\n", world_rank, offset, limit);
  //   readPoints(pathDataset, &pts, dim, offset, limit);
  // }
  // else{
  //   offset = numPt/3*2;
  //   limit = numPt-(numPt/3*2);
  //   printf("Rank %d: offset = %d, limit= %d\n", world_rank, offset, limit);
  //   readPoints(pathDataset, &pts, dim, offset, limit);
  // }



  for (int i = 0; i < numPt; i++)
    {
        int permissible = 1;

        for (int j = 0; j < numPt; j++)
        {
            if(i==j)
                continue;
            int counter = 0;
            
            for (int k = 0; k < dim; k++)
            {   
                if (pts[i].values[k] >= pts[j].values[k]){
                    counter++;
                }
                else break;
                
            }

            if (counter==dim){
                permissible=0;
                break;
            } 
            
        }

        if(permissible){
            memcpy(&pPermissiblePoints[permissiblePointNum++],&pts[i], sizeof(Point));
        }
        
    }


  printf("final permissiblePointNum = %d\n",permissiblePointNum);
  

  if(world_rank ==0) 
  writePermissiblePtToFile(pPermissiblePoints, permissiblePointNum, pathOutput); //write the result to file 'pathOutput'
  
  MPI_Finalize();
  return 0;
}
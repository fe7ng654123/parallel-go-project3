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

typedef struct point_max_dim{
    int ID;
    float values[5];
} Point_Max;

int get_p_points(Point *pts, int split_size, int dim, Point *p_result, int* p_num){

  for (int i = 0; i < split_size; i++)
  {
      int permissible = 1;

      for (int j = 0; j < split_size; j++)
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
          memcpy(&p_result[(*p_num)++],&pts[i], sizeof(Point));
          // p_result[(*p_num)++] = pts[i].ID;
      }
      
  }

}


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

  int permissiblePointNum = 0;
  Point* pPermissiblePoints = NULL;

  pPermissiblePoints = malloc((numPt+1)/3*sizeof(Point));
  // pPermissiblePoints = realloc(pPermissiblePoints, (numPt+1)/3*sizeof(Point));

  MPI_Aint displacements[2]  = {offsetof(Point_Max, ID), offsetof(Point_Max, values)};
  int block_lengths[2]  = {1,5};
  MPI_Datatype types[2] = {MPI_INT, MPI_FLOAT};
  MPI_Datatype custom_dt;

  // 2- Create the type, and commit it
  MPI_Type_create_struct(2, block_lengths, displacements, types, &custom_dt);
  MPI_Type_commit(&custom_dt);

  Point* pts;
  int offset = 0;                   //skip that many points before beginning to return points in readPoints()
  int limit = numPt;                //number of points to return in readPoints()

  // readPoints(pathDataset, &pts, dim, offset, limit);  //Read points from file 'pathDataset' and store in 'pts'

  if(world_rank == 0){
    offset = 0;
    limit= (numPt+1)/3;
  }
  else if (world_rank ==1){
    offset = (numPt+1)/3;
    limit = (numPt+1)/3;
  }
  else{
    offset = (numPt+1)/3*2;
    limit = numPt-((numPt+1)/3*2);
  }

  // printf("Rank %d: offset = %d, limit= %d\n", world_rank, offset, limit);
  readPoints(pathDataset, &pts, dim, offset, limit);


  int split_size = limit;
  // printf("Rank %d : split_size=%d\n",world_rank,split_size);

  

  get_p_points(pts,split_size,dim,pPermissiblePoints,&permissiblePointNum);

  printf("Rank %d : permissiblePointNum = %d\n",world_rank,permissiblePointNum);

  Point_Max* gathered_p_pts = NULL;
  Point_Max* p_result = NULL;
  // int* gathered_ids = malloc((numPt+1)/3*sizeof(int));
  // int* gathered_ids =NULL;
  int *counts = malloc(world_size*sizeof(int));
  int gathered_num = 0;
  int count_tag = 0;

  MPI_Gather(&permissiblePointNum, 1, MPI_INT, counts, 1, MPI_INT, 0, MPI_COMM_WORLD);
 

  for (int i = 0; i < world_size && world_rank == 0; i++)
  {
    // printf("counts[%d] = %d\n",i,counts[i]); 
    gathered_num += counts[i];
  }
  // gathered_ids = malloc(gathered_num*sizeof(int));
  
  gathered_p_pts = malloc(gathered_num*sizeof(Point_Max));

  int disps[3] = {0, counts[0], counts[0]+counts[1]};

  p_result=malloc(permissiblePointNum*sizeof(Point_Max));

  for (int i = 0; i < permissiblePointNum; i++)
  {
    printf("gathered_num = %d\n",gathered_num);
    p_result[i].ID=pPermissiblePoints[i].ID;
    for (int j = 0; j < dim; j++)
    {
      p_result[i].values[j]=pPermissiblePoints[i].values[j];
    }
    // printf("p_result[%d].ID = %d\n",i,p_result[i].ID);
    
  }
  
  
  MPI_Gatherv(p_result, permissiblePointNum, custom_dt,
              gathered_p_pts, counts,disps, custom_dt, 0, MPI_COMM_WORLD);

  
  if(world_rank == 0){
    free(pts);
    pts = malloc(gathered_num*sizeof(Point));

    for (int i = 0; i < gathered_num; i++)
    {
      pts[i].values = malloc(dim*sizeof(float));
      pts[i].ID = gathered_p_pts[i].ID;
      for (int j = 0; j < dim; j++)
      {
        pts[i].values[j]=gathered_p_pts[i].values[j];
      }
    }
    
    permissiblePointNum = 0;
    get_p_points(pts,gathered_num,dim,pPermissiblePoints,&permissiblePointNum);
    printf("final permissiblePointNum = %d\n",permissiblePointNum);
    writePermissiblePtToFile(pPermissiblePoints, permissiblePointNum, pathOutput);
  }
  
  
  // writePermissiblePtToFile(pPermissiblePoints, permissiblePointNum, pathOutput); //write the result to file 'pathOutput'
  
  MPI_Finalize();
  return 0;
}
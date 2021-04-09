#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<CUnit/Basic.h>
#include<sys/time.h>
#include"util.h"

Point *generateDataset(int number, int dim, float arg, int seed);

int *checkPermissiblePointID = NULL; //
int permissiblePointNum = 0;
int checkPermissiblePointNum = 0;
int iter = 0;
int numNode = 1;

typedef struct testcase {
  int number;
  int dim;
  float arg;
  int seed;
  int threads;
  char result[100];
  char testcase[64];
} Testcase;

Testcase *testcases;

int readPP(char* fileName, int** ptIds){
  FILE *file = fopen(fileName, "r");
  int retNumPt;
  if (file == NULL) {
    printf("cannot open %s\n", fileName);
    return -1;
  }

  int n = fscanf(file, "%d", &retNumPt);
  if (n != 1) {
    printf("READ ERROR: cannot read the number of permissible points in %s\n", fileName);
    return -1;
  }
  (*ptIds) = (int *) malloc(sizeof(int) * retNumPt);
  if ((*ptIds) == NULL) {
    printf("CAN'T allocate memory for checkPermissiblePointID array!\n");
    return -1;
  }

  for (int i = 0; i < retNumPt; i++) {
    n = fscanf(file, "%d", &(*ptIds)[i]);
    if (n != 1) {
      printf("READ ERROR: cannot read checkPermissiblePointID[%d]\n", i);
      return -1;
    }
  }
  return retNumPt;
}

int init_test(Testcase test) {
//	Testcase test = testcases[iter];
//  points = generateDataset(test.number, test.dim, test.arg, test.seed);
//  if points == NULL) {
//    printf("cannot generate points\n");
//    return -1;
//  }
//
  checkPermissiblePointNum = readPP(test.result, &checkPermissiblePointID);
  if (checkPermissiblePointNum == -1){
    printf("Fail when reading the expected result");
  }
  return 0;

}

// It is used for cleaning the pointers.
int clean_test(void) {
  if (checkPermissiblePointID != NULL)
    free(checkPermissiblePointID);
  checkPermissiblePointID = NULL;
  checkPermissiblePointNum = 0;

  permissiblePointNum = 0;
  iter++;  // Here we set iter ++, because we want to go next test case.
  return 0;
}

int init_suite(void) { return 0; }
int clean_suite(void) { return 0; }

void execGenerateDataset(int numPt,
                         int dim,
                         float arg,
                         int seed,
                         char *dstFileName,
                         char *pathGenerateDataset) {
  int cpid = fork();
  if(cpid == 0) {
    //child
    char buf1[20];
    char buf2[20];
    char buf3[20];
    char buf4[20];
    sprintf(buf1, "%d", numPt);
    sprintf(buf2, "%d", dim);
    sprintf(buf3, "%f", arg);
    sprintf(buf4, "%d", seed);
    execl(pathGenerateDataset, "generateDataset", buf1, buf2, buf3, buf4, dstFileName, NULL);
    perror("Fail to generate dataset");
    exit(0);
  }
  else {
    //parent
    wait(NULL);
  }
}

void execYourMPIProgram(char* pathBinYourProgram, int numPt, int dim, char* pathDataset, char* pathOutput, int numNode){
  int cpid = fork();
  if(cpid == 0) {
    char cmd[200];
    sprintf(cmd,
            "./%s %d %d %s %s",
            pathBinYourProgram,
            numPt,
            dim,
            pathDataset,
            pathOutput);

    if(numNode > 1){
      char sallocCmd[200];
      sprintf(sallocCmd,
          "salloc -N %d -n %d mpirun %s",
          numNode,
          numNode,
          cmd);
      execl("/bin/bash", "bash", "-c", sallocCmd, NULL);
    }else{
      execl("/bin/bash", "bash", "-c", cmd, NULL);
    }
    perror("Can't run your mpi program");
  }else{
    //parent
    wait(NULL);
  }
}

int cmpID(const void *a, const void *b){
  return *(int*)a - *(int*)b;
}


int testCase(void) {
  struct timeval t_start, t_end;
  int threadNum = 1;
  Testcase test = testcases[iter];

  int status = init_test(test);

  if (status == -1) {
    clean_test();
    return 0;
  }

  //Generate dataset
  //Assume the working directory is in shared storage
  char* pathDataset = "dataset.pts";
  char* pathBinGenerateDataset ="./generateDataset";
  execGenerateDataset(test.number,
                      test.dim,
                      test.arg,
                      test.seed,
                      pathDataset,
                      pathBinGenerateDataset);

  // execute the student part
  gettimeofday(&t_start, NULL);
  //permissiblePointNum = asgn2a(points, &permissiblePoints, test.number, test.dim, threadNum);
  char * pathBinYourProgram ="asgn2c";
  char * outputFilePath = "ret.pts";

  execYourMPIProgram(pathBinYourProgram, test.number, test.dim, pathDataset, outputFilePath, numNode);

  gettimeofday(&t_end, NULL);

  int * permissiblePointIds = NULL;
  permissiblePointNum = readPP(outputFilePath, &permissiblePointIds);
  double t_ep = (t_end.tv_sec - t_start.tv_sec) * 1000000.0 + t_end.tv_usec - t_start.tv_usec;
  t_ep = t_ep / 1000000.0;
  printf("Time cost: %.4lfs\n", t_ep);

  if (permissiblePointIds != NULL && permissiblePointNum != 0)
    qsort(permissiblePointIds, permissiblePointNum, sizeof(int), cmpID);

//  for(int x=0; x<permissiblePointNum;x++){
//    printf("%d\n", permissiblePointIds[x]);
//  }
  if (permissiblePointNum != checkPermissiblePointNum) {
    clean_test();
    return 0;
  } else {
    for (int i = 0; i < checkPermissiblePointNum; i++) {  // Why we need to use -std=c99
      if (permissiblePointIds[i] != checkPermissiblePointID[i]) {
        clean_test();
        return 0;
      }
    }

    clean_test();
    if(permissiblePointIds != NULL)
      free(permissiblePointIds);

//    char cmdRm[100];
//    sprintf(cmdRm, "rm %s %s", pathDataset, outputFilePath);
//    execl("/bin/bash", "bash", "-c", cmdRm);
    return 1;
  }
}


void test(void) {
  CU_ASSERT(testCase());
}


int main(int agrc, char* argv[]){

  if(agrc != 2){
    printf("Usage: %s <test case no.>\n\n"
           "\ttest case no.:\n"
           "\t\t'0' to run a toy test case;\n"
           "\t\t'1'~'12' to select a test case;\n"
           "\t\t\'13' to run all the 13 test cases\n\n"

//           "\tnumber of server:\n"
//           "\t\t'1' to run on without mpiu; \n"
//           "\t\t>1 to run with slurm and mpirun with that number of server\n"
            , argv[0]);
    return -10;
  }

  int select = atoi(argv[1]);
  numNode = 3;

  FILE * fr = fopen("./testcase/testsuite.txt","r");
  if( fr == NULL ){
    printf("ERROR: cannot open the inputfile.txt file.\n");
    exit(1);
  }
  int N;
  CU_pTest *tests = (CU_pTest*) malloc(sizeof(CU_pTest) * 13);
  testcases = (Testcase*)malloc(sizeof(Testcase) * 13);

  for(int i = 0; i < 13; i ++) {
    N = fscanf(fr, "%d %d %f %d %d %s", &testcases[i].number, &testcases[i].dim,
               &testcases[i].arg, &testcases[i].seed, &testcases[i].threads, testcases[i].result);

    if(i == 0){
      sprintf(testcases[i].testcase, "toyTestCase");
    } else {
      sprintf(testcases[i].testcase, "testcase%d", i);
    }
  }

  CU_pSuite pSuite = NULL;
  /* initialize the CUnit test registry */
  if(CUE_SUCCESS != CU_initialize_registry())
    return CU_get_error();

  /* add a suite to the registry */
  pSuite = CU_add_suite("SUITE", init_suite, clean_suite);
  if(NULL == pSuite ){
    CU_cleanup_registry();
    return CU_get_error();
  }

  for(int i = 0; i < 13; i++){
    if((tests[i] = CU_add_test(pSuite, testcases[i].testcase, test)) == NULL) {
      CU_cleanup_registry();
      return CU_get_error();
    }
  }


  CU_basic_set_mode(CU_BRM_VERBOSE);

  if( select == 13 ){
    CU_basic_run_tests();
  } else if(select >=0 && select <= 12) {
    iter += select;
    CU_basic_run_test(pSuite, tests[select]);
  } else {
    printf("Please type a number of '0' ~ '13'!\n");
    return 0;
  }

  CU_cleanup_registry();

//	printf("Finally, your score is %d!\n", score);

  free(testcases);
  return CU_get_error();
}

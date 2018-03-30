#include <stdio.h>
#include <stdlib.h>

#include <string.h>
#include <signal.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>

#include "sudoku.h"
#include "include/easy_solve.h"
#include "include/hard_solve.h"

struct cp_thread_data {
  int thread_id;
  SuDoku *board;
  board_constraint *constraint;
  c_address curCell;
};

/* Master array of threads */
struct cp_thread_data cp_thread_data_array[9];

void printSummary( void );
void scanConstraintsPthread(SuDoku);

/* Timers for total run time */
struct timeval real_start, real_end;

/* Benchmark info */
int benchmark = 0;

/******************************************************************************/
void printUsage( ) {
  printf ("\nUsage: sudoku [-hbptc] [-f file]\n");
  printf ("	-h	Print this help message\n");
  printf ("	-f file	Use input file instead of default 'board.txt'\n");
  printf ("	-t	Use multithreading\n");
  printf ("	-b	Benchmark, input file should be Top95 format\n");
  printf ("	-p	Print the unfinished board and exit\n");
  printf ("	-c	Print the constraints of the board and exit\n");
  printf ("\n");
}
/******************************************************************************/

/******************************************************************************/
void solutionFunc(SuDoku *board) {
/******************************************************************************/

  gettimeofday(&real_end, NULL);

  if (benchmark == 0) {
    printBoard(board);
    printSummary();
  } else {
    printBoardBenchmark(board);
  }

  exit(0);
}

/******************************************************************************/
void *pthread_callConstraintPropagation( void *threadarg) {
/******************************************************************************/
  struct cp_thread_data *my_data;

  my_data = (struct cp_thread_data *) threadarg;

  scanBoard(my_data->board, my_data->constraint);
  updateConstraint(my_data->board, my_data->constraint);

  if (benchmark == 0) {
    printf("Thread %d (%d) starting at: ", my_data->thread_id, getpid());
    printf("(%d, %d)", my_data->curCell.section, my_data->curCell.cell);
    printf(" => %d\n", my_data->curCell.value);
  }

  constraintPropagation (my_data->board, my_data->constraint, my_data->curCell, solutionFunc);
  return NULL;
}

/******************************************************************************/
int main (int argc, char *argv[]) {
/******************************************************************************/
  SuDoku Board;
  board_constraint Constraint;

  int print_board = 0;
  int print_constraints = 0;
  int thread = 0;

  FILE *fin;
  pid_t child_pid;
  
  extern char *optarg;				/* Operand variables */
  extern int optind, optopt, opterr;
  char *filename = "board.txt";
  int opt;

  gettimeofday(&real_start, NULL);

 /* Get operands */
  while ((opt = getopt(argc, argv, ":hbpcts:d:f:")) != -1) {
    switch(opt) {
      case 'h':
        printUsage();
        exit(0);

      case 'b':
        benchmark = 1;
        break;

      case 'p':
        print_board = 1;
        break;

      case 'c':
        print_constraints = 1;
        break;

      case 't':
        thread = 1;
        break;

      case 'f':
        filename = optarg;
        break;
    }
  }

  if (benchmark == 1) {
    if (strncmp("-", filename, 1) == 0) {
      fin = fdopen(dup(STDIN_FILENO), "r");
    } else {
      fin = fopen(filename, "rb");
    }
    if ( fin == NULL ) {
      printf("Error: could not open '%s'\n", filename);
      exit(1);
    }

    printf("\n");
    while (loadBenchmark(&Board,fin) == 0) {
      child_pid = fork();
      if (child_pid == 0) {
        fclose(fin);
        if (thread == 0) {
          resetConstraint(&Constraint);
          updateConstraint(&Board, &Constraint);

          constraintPropagation (&Board, &Constraint, findLeastConstrained(&Board, &Constraint), solutionFunc);
        } else {
          scanConstraintsPthread(Board);
        }
        exit(0);
      } else {
        waitpid(child_pid, NULL, 0);
        benchmark++;
      }
    }

    benchmark = 0;
    gettimeofday(&real_end, NULL);

    printSummary();
    exit(0);
  }

  resetBoard(&Board);
  loadBoard(&Board, filename);

  resetConstraint(&Constraint);
  updateConstraint(&Board, &Constraint);

  if (print_board == 1 || print_constraints == 1) {
    if (print_board == 1) { printBoard(&Board); }
    if (print_constraints == 1) { printConstraint(&Board, &Constraint); }

    exit(0);
  }

  if (thread == 0) {
    constraintPropagation (&Board, &Constraint, findLeastConstrained(&Board, &Constraint), solutionFunc);
  } else {
    scanConstraintsPthread(Board);
  }
  return(0);
}

/******************************************************************************/
void scanConstraintsPthread(SuDoku Board) {
/******************************************************************************/
  SuDoku *newBoard;
  board_constraint Constraint, *newConstraint;
  int j,k;

  pthread_t thread[9];
  pthread_attr_t attr;
  int rc, status;

  c_address listMove[9];

 /* Set up constraint object */
  resetConstraint(&Constraint);
  updateConstraint(&Board, &Constraint);

  if ( scanBoard(&Board, &Constraint) != 0 ) {
   /*
    * If an initial scan does not solve the board then begin with the brute forcing.
    */

    listMove[0] = findLeastConstrained(&Board, &Constraint);

   /* Get all the available cells for the current value and the first possible section */
    k=1;
    listMove[k] = nextConstraint (&Constraint, listMove[k-1]);
    while (listMove[k].section == listMove[k-1].section && listMove[k].cell == listMove[k-1].cell) {
      k++;
      listMove[k] = nextConstraint (&Constraint, listMove[k-1]);
    }

   /* Break up the brute forcing into one thread for each possible first position */
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

    for (j=0; j<k; j++) {
      newBoard = malloc(sizeof(SuDoku));
      memcpy(newBoard, &Board, sizeof(SuDoku));

      newConstraint = malloc(sizeof(board_constraint));
      memcpy(newConstraint, &Constraint, sizeof(board_constraint));

      newBoard->section[ listMove[j].section ].cell[ listMove[j].cell ] = listMove[j].value;
      if (scanBoard(newBoard, newConstraint) == 0) {
        gettimeofday(&real_end, NULL);

        if ( benchmark == 0 ) {
          printBoard(newBoard);
          printSummary();
        } else {
          printBoardBenchmark(newBoard);
        }

        exit(0);
      }

      updateConstraint(newBoard, newConstraint);
      if (checkConstraintRules(newBoard, newConstraint) == 0) {
        listMove[j] = findLeastConstrained(newBoard, newConstraint);

        cp_thread_data_array[j].thread_id = j;
        cp_thread_data_array[j].board = newBoard;
        cp_thread_data_array[j].curCell = listMove[j];
        cp_thread_data_array[j].constraint = newConstraint;

        rc = pthread_create(&thread[j], &attr, pthread_callConstraintPropagation, &cp_thread_data_array[j]);
        if (rc) { printf("Error: return code from pthread_create() is %d\n",rc); exit(-1); }
      } else {
        thread[j] = 0;
      }
    }

    pthread_attr_destroy(&attr);
    for (j=0; j<k; j++) {
      if (thread[j]) {
        rc = pthread_join(thread[j], (void **)&status);
        if (rc) { printf("Error: return code from pthread_join() of %d is %d\n", j, rc);
          printf("EDEADLK: %d, EINVAL: %d, ESRCH: %d\n", EDEADLK, EINVAL, ESRCH);
          exit(-1);}
      }
    }
    exit(0);

  }

  gettimeofday(&real_end, NULL);

  if ( benchmark == 0 ) {
    printBoard(&Board);
    printSummary();
  } else {
    printBoardBenchmark(&Board);
  }

}

/******************************************************************************/
void printSummary( void ) {
/******************************************************************************/
  float total_sec, total_usec, total_time;

  total_sec = real_end.tv_sec - real_start.tv_sec;
  total_usec = real_end.tv_usec - real_start.tv_usec;

  total_time = total_sec + (total_usec / 1000000);

  if ( benchmark == 0 ) {
    printf("\nTotal Time: %.3f sec\n\n", total_time);
  } else {
    printf("\t%d Benchmark Time: %.3f sec\n", benchmark, total_time);
  }
}


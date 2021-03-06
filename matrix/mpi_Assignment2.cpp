# include <cmath>
# include <mpi.h>
# include <iostream>
# include <cstdlib>
# include <ctime>

using namespace std;

int main ( int argc, char *argv[] );
void timestamp ( );

/******************************************************************************/

int main ( int argc, char *argv[] )

/******************************************************************************/
/*
  Purpose:

    MAIN is the main program for MATVEC.

  Discussion:

    MATVEC uses MPI to compute a matrix-vector product b = A * x.

    This is the simple self-scheduling version.  Each worker is given 
    a copy of x, and then is fed one row of A.  As soon as it computes 
    b(I) = A(I,1:N)*x(1:N), it is given another column of A, unless
    there are no more, in which case it is sent a "terminate" message. 
    Thus, a faster process will be given more work to do.

    By using allocatable arrays, the amount of memory used has been
    controlled.  The master process allocates A and x, but the worker
    processes only allocate enough memory for one row of A, and x.

  Licensing:

    This code is distributed under the GNU LGPL license. 

  Modified:

    11 October 2002

  Author:

    John Burkardt

  Reference:

    William Gropp, Ewing Lusk, Anthony Skjellum,
    Using MPI: Portable Parallel Programming with the
    Message-Passing Interface,
    Second Edition,
    MIT Press, 1999,
    ISBN: 0262571323.

    Snir, Otto, Huss-Lederman, Walker, Dongarra,
    MPI - The Complete Reference,
    Volume 1, The MPI Core,
    second edition,
    MIT Press, 1998.
*/
{
  double *a;
  double *a_row;
  double ans;
  double *b;
  int dest;
  int dummy;
  int i;
  int ierr;
  int j;
  int j_one;
  int k;
  int m;
  int master = 0;
  int my_id;
  int n;
  int num_procs;
  int num_rows;
  int num_workers;
  double pi = 3.141592653589793;
  MPI_Status status;
  int tag;
  int tag_done;
  double *x;
/*
  Initialize MPI.
*/
  ierr = MPI_Init ( &argc, &argv );

  if ( ierr != 0 )
  {
    cout << "\n";
    cout << "MATVEC_MPI - Fatal error!\n";
    cout << "  MPI_Init returns nonzero IERR.\n";
    exit ( 1 );
  }
/*
  Get this processor's ID.
*/
  ierr = MPI_Comm_rank ( MPI_COMM_WORLD, &my_id );
/*
  Get the number of processors.
*/
  ierr = MPI_Comm_size ( MPI_COMM_WORLD, &num_procs );

  if ( my_id == 0 ) 
  {
    timestamp ( );
    cout << "\n";
    cout << "MATVEC - Master process:\n";
    cout << "  C++ version\n";
    cout << "  An MPI example program to compute\n";
    cout << "  a matrix-vector product b = A * x.\n";
    cout << "\n";
    cout << "  Compiled on " << __DATE__ << " at " << __TIME__ << ".\n";
    cout << "\n";
    cout << "  The number of processes is num_procs.\n";
  }
  cout << "\n";
  cout << "Process " << my_id << " is active.\n";

  m = 4;
  n = 4;
  tag_done = m + 1;

  if ( my_id == 0 ) 
  {
    cout << "\n";
    cout << "  The number of rows is " << m << ".\n";
    cout << "  The number of columns is " << n << ".\n";
  }
/*
  The master process allocates and initializes A and X.

  Because we are dynamically allocating A, we can't use 2D array double
  indexing, so we have to figure out where we are on our own.
*/
  if ( my_id == master )
  {
    a = ( double * ) malloc ( m * n * sizeof ( double ) );
    x = ( double * ) malloc ( n * sizeof ( double ) );
    b = ( double * ) malloc ( m * sizeof ( double ) );

    k = 0;
    for ( i = 1; i <= m; i++ ) 
    {
      for ( j = 1; j <= n; j++ )
      {
        a[k] = sqrt ( 2.0 / ( double ) ( n + 1 ) ) 
          * sin ( ( double ) ( i * j ) * pi / ( double ) ( n + 1 ) );
        k = k + 1;
      }
    }
/*
  X is specially chosen so that b = A * x is known in advance.
  The value of b will be zero, except that entry J_ONE will be 1.
  Pick any value of J_ONE between 1 and M.
*/
    j_one = 2;
    for ( i = 0; i < n; i++ )
    {
      x[i] = sqrt ( 2.0 / ( double ) ( n + 1 ) ) 
        * sin ( ( double ) ( ( i + 1 ) * j_one ) * pi / ( double ) ( n + 1 ) );
    }

    cout << "\n";
    cout << "MATVEC - Master process:\n";
    cout << "  Vector x:\n";
    cout << "\n";
    for ( i = 0; i < n; i++ )
    {
      cout << i << " " << x[i] << "\n";
    }

  }
/*
  Worker processes set aside room for one row of A, and for the 
  vector X.
*/
  else
  {
    a_row = ( double * ) malloc ( n * sizeof ( double ) );
    x = ( double * ) malloc ( n * sizeof ( double ) );
  }
/*
  Process 0 broadcasts the vector X to the other processes.
*/
  ierr = MPI_Bcast ( x, n, MPI_DOUBLE, master, MPI_COMM_WORLD );

  if ( my_id == master )
/*
  Process 0 sends one row of A to all the other processes.

  If we were using standard C 2D array storage, the entries of
  the row would be contiguous; using pointers, we have ended up
  in the same situation.  As long as the entries are contiguous,
  we can use a simple standard datatype with MPI_Send.  

  The situation would require a little more work if we tried
  to send a column of data instead of a row.
*/
  {
    num_rows = 0;

    for ( i = 1; i <= num_procs-1; i++ )
    {
      dest = i;
      tag = num_rows;
      k = num_rows * n;

      ierr = MPI_Send ( a+k, n, MPI_DOUBLE, dest, tag, MPI_COMM_WORLD );

      num_rows = num_rows + 1;
    }
     
    num_workers = num_procs-1;

    for ( ; ; )
    {
      ierr = MPI_Recv ( &ans, 1, MPI_DOUBLE, MPI_ANY_SOURCE,
        MPI_ANY_TAG, MPI_COMM_WORLD, &status );

      tag = status.MPI_TAG;
      b[tag] = ans;

      if ( num_rows < m )
      {
        num_rows = num_rows + 1;
        dest = status.MPI_SOURCE;
        tag = num_rows;
        k = num_rows * n;

        ierr = MPI_Send ( a+k, n, MPI_DOUBLE, dest, tag, MPI_COMM_WORLD );
      }
      else
      {
        num_workers = num_workers - 1;
        dummy = 0;
        dest = status.MPI_SOURCE;
        tag = tag_done;

        ierr = MPI_Send ( &dummy, 1, MPI_INT, dest, tag, MPI_COMM_WORLD );

        if ( num_workers == 0 )
        {
          break;
        }
      }

    }

    free ( a );
    free ( x );
  }
/*
  Each worker process repeatedly receives rows of A (with TAG indicating
  which row it is), computes dot products A(I,1:N) * X(1:N) and returns
  the result (and TAG), until receiving the "DONE" message.
*/
  else
  {
    for ( ; ; )
    {
      ierr = MPI_Recv ( a_row, n, MPI_DOUBLE, master, MPI_ANY_TAG,
        MPI_COMM_WORLD, &status );

      tag = status.MPI_TAG;

      if ( tag == tag_done ) 
      {
        cout << "  Process " << my_id << " shutting down.\n";
        break;
      }

      ans = 0.0;
      for ( i = 0; i < n; i++ )
      {
        ans = ans + a_row[i] * x[i];
      }

      ierr = MPI_Send ( &ans, 1, MPI_DOUBLE, master, tag, MPI_COMM_WORLD );

    }

    free ( a_row );
    free ( x );
  }
/*
  Print out the answer.
*/
  if ( my_id == master ) 
  {
    cout << "\n";
    cout << "MATVEC - Master process:\n";
    cout << "  Product vector b = A * x\n";
    cout << "  (Should be zero, except for a 1 in entry " << j_one-1 << "\n";
    cout << "\n";
    for ( i = 0; i < m; i++ )
    {
      cout << i << " " << b[i] << "\n";
    }

    free ( b );
  }
/*
  Terminate MPI.
*/
  ierr = MPI_Finalize ( );
/*
  Terminate.
*/
  if ( my_id == master ) 
  {
    cout << "\n";
    cout << "MATVEC - Master process:\n";
    cout << "  Normal end of execution.\n";
    cout << "\n";
    timestamp ( );
  }
  return 0;
}
/******************************************************************************/

void timestamp ( )

/******************************************************************************/
/*
  Purpose:

    TIMESTAMP prints the current YMDHMS date as a time stamp.

  Example:

    31 May 2001 09:45:54 AM

  Licensing:

    This code is distributed under the GNU LGPL license. 

  Modified:

    24 September 2003

  Author:

    John Burkardt

  Parameters:

    None
*/
{
# define TIME_SIZE 40

  static char time_buffer[TIME_SIZE];
  const struct tm *tm;
  time_t now;

  now = time ( NULL );
  tm = localtime ( &now );

  strftime ( time_buffer, TIME_SIZE, "%d %B %Y %I:%M:%S %p", tm );

  cout << time_buffer << "\n";

  return;
# undef TIME_SIZE
}

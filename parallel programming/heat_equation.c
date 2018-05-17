#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <math.h>

const float L = 1.0;
const float T0 = 1.0;
const float T1 = 0.0;
const float T2 = 0.0;
const float k = 1;
const float inf = 3;
const double pi = 3.14159265359;

float f(float x, float t);

int main(int argc, char **argv)
{
        if (argc < 3) {
                printf("should be 2 arguments\n");
        }
        int N = atoi(argv[1])+2;
        double T = atof(argv[2]);
        double *u1, *u2;
        double h = L/(N-2);
        double dt = h*h/(2.0*k);
        int steps = T/dt;
        int i, j;
        int rank, size;
        double begin, end;

        MPI_Status status;
        MPI_Init(&argc, &argv);
        MPI_Comm_size(MPI_COMM_WORLD, &size);
        MPI_Comm_rank(MPI_COMM_WORLD, &rank);

        /* Выделение памяти. */
        /* Выделение памяти. */
        u1 = (double*)calloc(N, sizeof(double));
        u2 = (double*)calloc(N, sizeof(double));

        /* Граничные условия. */
        u1[0] = u1[1] = T1;
        u1[N - 1] = u1[N-2] = T2;
        for (i = 2; i < N-2; i++) u1[i] = T0;
        for (i = 0; i < N; i++) u2[i] = u1[i];

        /*Последовательный проход*/
        if (rank == 0){
                begin = MPI_Wtime();
                for (i = 0; i < steps; i++) {
                        for (j = 2; j < N-1; j++) {
                                u2[j] = u1[j] + k*dt*(u1[j-1] - 2.0*u1[j] + u1[j+1])/(h*h);
                        }
                        double *t = u1;
                        u1 = u2;
                        u2 = t;
                }
                end = MPI_Wtime();
                int i = 0;
                while (i < N-1){
                        printf("%f %f %f\n", h * i, u1[i+1], f(h*i, T));
                        i += (N-1)/10;
                }
                printf("sequential transit time = %f\n", end - begin);
                int wait = 1;
                for (i = 1; i < size; i++)
                        MPI_Send(&wait,1,MPI_INT,i,1,MPI_COMM_WORLD);
        }

        if(rank != 0){
                int wait = 0;
                MPI_Recv(&wait,1,MPI_INT,0,1,MPI_COMM_WORLD,&status);
        }

        /*Граничные условия*/
        u1[0] = u1[1] = T1;
        u1[N - 1] = u1[N-2] = T2;
        for (i = 2; i < N-2; i++) u1[i] = T0;
        for (i = 0; i < N; i++) u2[i] = u1[i];

        int perproc = N / size;  // количество участков на каждый процесс
        int st = perproc * rank; // начало каждого процесса
        int fn = st + perproc;   // конец каждого процесса
        if (rank == size - 1) fn = N - 1;
        if (rank == 0) st = 2;

        /*Параллельный проход*/
        begin = MPI_Wtime();
        for (i = 0; i < steps; i++){
                if (rank%2 == 0){
                        if (rank < (size-1)){
                                MPI_Ssend(&u1[fn-2], 2, MPI_DOUBLE, rank+1, 1, MPI_COMM_WORLD);
                                MPI_Recv(&u1[fn], 2, MPI_DOUBLE, rank+1 , 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                                }
                        if (rank>0){
                                MPI_Ssend(&u1[st], 2, MPI_DOUBLE, rank-1, 1, MPI_COMM_WORLD);
                                MPI_Recv(&u1[st-2], 2, MPI_DOUBLE, rank-1 , 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                        }
                }
                else{
                        if (rank > 0){
                                MPI_Recv(&u1[st-2], 2, MPI_DOUBLE, rank-1 , 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                                MPI_Ssend(&u1[st], 2, MPI_DOUBLE, rank-1, 1, MPI_COMM_WORLD);
                        }
                        if (rank < (size-1)){
                                MPI_Recv(&u1[fn], 2, MPI_DOUBLE, rank+1 , 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                                MPI_Ssend(&u1[fn-2], 2, MPI_DOUBLE, rank+1, 1, MPI_COMM_WORLD);
                        }
                }

                for (j = st; j < fn; j++) {
                        u2[j] = u1[j] + k*dt*(u1[j-1] - 2.0*u1[j] + u1[j+1])/(h*h);
                }
                double *t = u1;
                u1 = u2;
                u2 = t;
        }
        if (rank == 0) {
                for (i = 1; i < size; i++) {
                        MPI_Recv(u1 + perproc*i, perproc + (i == size-1 ? N%size : 0), MPI_DOUBLE, i, 0, MPI_COMM_WORLD, &status);
                }
                i = 0;
                end = MPI_Wtime();
                while (i < N-1){
                        printf("%f %f %f\n", h*i, u1[i+1], f(h*i, T));
                        i += (N-1)/10;
                }
                printf("parallel transit time = %f\n", end - begin);
        }
        else{
                MPI_Send(u1 + perproc*rank, perproc + (rank == size-1 ? N%size : 0), MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);
        }

        free(u1);
        free(u2);
        MPI_Finalize();
        return 0;
}

float f(float x, float t){
        int i = 0;
        double sum = 0;
        for (i = 0; i < inf; i++){
                sum += exp(-k*pi*pi*(2*i+1)*(2*i+1)*t/(L*L))/(2*i+1)*sin(pi*(2*i+1)*x/L);
        }
        return 4*T0*sum/pi;
}


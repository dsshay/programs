#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

const int N = 10;

double integral (int a, int N,  double h);
double f(double x);

int main(int argc, char* argv[]){

        int myrank, size;
        MPI_Status Status;
        MPI_Init(&argc, &argv);
        MPI_Comm_size(MPI_COMM_WORLD, &size);
        MPI_Comm_rank(MPI_COMM_WORLD, &myrank);

        double h = 1.0/N;


        double Sum, begin1, end1, total1, begin2, end2, total2;
        int i, launch = 0, completion = 0;
        double part_ammount[size];
        if(myrank == 0){
                begin1 = MPI_Wtime();
                printf("Integral is equal to %.10f\n", integral(0, N, h));
                end1 = MPI_Wtime();
                total1 = end1 - begin1;

                int part = (int)(N/size);

                begin2 = MPI_Wtime();
                for (i = 1; i < size; i++){
                        MPI_Send(&launch,1,MPI_INT,i,1,MPI_COMM_WORLD);
                        MPI_Send(&h, 1,MPI_DOUBLE,i,1,MPI_COMM_WORLD);
                        if(i <= N % size){
                                launch += ++part;
                                completion = launch;
                                MPI_Send(&completion,1,MPI_INT,i,1,MPI_COMM_WORLD);
                        }
                        else {
                                launch += part;
                                completion = launch;
                                MPI_Send(&completion, 1, MPI_INT, i,1,MPI_COMM_WORLD);
                        }
                }

        }
        else {
                MPI_Recv(&launch,1,MPI_DOUBLE,0,1,MPI_COMM_WORLD,&Status);
                MPI_Recv(&h,1,MPI_DOUBLE,0,1,MPI_COMM_WORLD, &Status);
                MPI_Recv(&completion,1,MPI_DOUBLE,0,1,MPI_COMM_WORLD, &Status);
                double ammount = integral (launch, completion, h);
                MPI_Send(&ammount, 1, MPI_DOUBLE, 0,2,MPI_COMM_WORLD);
        }

        if (myrank == 0){
                part_ammount[0] = integral(launch, N, h);
                for (i = 0; i < size; i++){
                        if (i > 0) MPI_Recv(&part_ammount[i], 1, MPI_DOUBLE, i,2,MPI_COMM_WORLD,&Status);
                        Sum += part_ammount[i];
                }
                end2 = MPI_Wtime();
                total2 = end2 - begin2;

                printf("Integral on parallel process is equal to %.10f\n", Sum);

                double acceleration = total1/total2;
                printf ("Acceleration(%d part, %d process) is equal to %f\n", N, size, acceleration);
        }

        MPI_Finalize();
return 0;
}


double integral(int a, int N, double h){
        int i;
        double x1, value;
        for (i = a; i < N; i++){
                x1 = i*h;
                value += 0.5*(f(x1+h) + f(x1))*h;
        }
return 4*value;
}

double f(double x){
return 1/(1 + x*x);
}


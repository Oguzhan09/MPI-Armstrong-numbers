/**
 * Student Name: Oğuzhan Kırlar
 * Student Number: 2014400195
 * Compile Status: Compiling
 * Program Status: Working
 */
#include <mpi.h>
#include <fstream>
#include <algorithm>
#include <iostream>
#include <random>
#include <chrono>

using namespace std;

int power(int x, unsigned int y) // this method calculates x over y
{
    if( y == 0)
        return 1;
    if (y%2 == 0)
        return power(x, y/2)*power(x, y/2);
    return x*power(x, y/2)*power(x, y/2);
}

int order(int x) // This function calculate order of the number
{
    int n = 0;
    while (x)
    {
        n++;
        x = x/10;
    }
    return n;
}


bool isArmstrong(int x) // Function to check whether the given number is Armstrong number or not
{
    int n = order(x); // Find order of x
    int temp = x, sum = 0;
    while (temp)
    {
        int r = temp%10;
        sum += power(r, n);
        temp = temp/10;
    }

    // If satisfies Armstrong condition
    if (sum == x){
        return true;
    }else{
        return false;
    }
}

void swap (int *a, int *b) // Function that displaces a and b
{
    int temp = *a;
    *a = *b;
    *b = temp;
}

void randomize (int arr[], int n) // Create randomly swapped new array
{
    srand (time(NULL));
    // Start from the last element and swap one by one.
    for (int i = n - 1; i > 0; i--)
    {
        // Pick a random index from 0 to i
        int j = rand() % (i + 1);
        // Swap arr[i] with the element at random index
        swap(&arr[i], &arr[j]);
    }
}

int main(int argc, char** argv) {
    auto start = chrono::steady_clock::now();
    // Initialize the MPI environment
    MPI_Init(NULL, NULL);
    // Find out rank, size
    int world_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    int N = world_size-1; //the number of slave processes.
    int total_sum=0;
    int array_size = atoi(argv[1]); // İnput for A
    int s = array_size/N; // array size for divided arrays
    if (world_rank == 0 ){ // if the master process
        int arr[array_size];
        for(int i = 0; i<array_size; i++){ // create array from 1 to A
            arr[i] = i+1;
        }
        randomize(arr, array_size); // randomize array
        int sending_array [s]; // create sending array
        for(int k = 1 ; k <= N ; ++k){  // split array
            for (int i = 0; i < s; i++) {
                sending_array[i] = arr[(k-1)*(s)+i];
            }
            MPI_Send(sending_array, s, MPI_INT, k, 0, MPI_COMM_WORLD); // send divided arrays to slave processors
        }
        for (int k = 1; k <= N; ++k) {  // from slave process 1 to N
            int a[s];
            MPI_Recv(a, s, MPI_INT, k, k, MPI_COMM_WORLD, MPI_STATUS_IGNORE); // receive armstrong numbers
        }
        int total = 0;
        MPI_Recv(&total, 1, MPI_INT, world_rank-1, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE); // receive total sum of armstrong numbers from last processor
        cout<<"MASTER: Sum of all Armstrong numbers = "<<total<<endl;
        auto end = chrono::steady_clock::now();
        cout << "Elapsed time in milliseconds : " << chrono::duration_cast<chrono::milliseconds>(end - start).count() << " ms" << endl; // if you uncommand this line, you can see the elapsed time
    }else{ // if slave processor
        int sub_arr[s];
        int sum = 0;
        MPI_Recv(sub_arr, s, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);  // receive divided array from master process
        for (int i =0; i<s; i++){  //control elements is armstrong or not
            if(!isArmstrong(sub_arr[i])){ // if the number is not armstrong number, replace the 0
                sub_arr[i] = 0;
            }else{
                sum += sub_arr[i]; // if armstrong add the sum
            }
        }
        MPI_Send(&sub_arr, s, MPI_INT, 0, world_rank, MPI_COMM_WORLD); // send master process the new sub array
        int new_sum = 0;
        if(world_rank == 1){ // if processor number is 1
            new_sum = new_sum + sum; // update sum
            MPI_Send(&new_sum, 1, MPI_INT, 2, world_rank, MPI_COMM_WORLD); // send processor 2
        }else if(world_rank == world_size-1){  // if processor number is world_rank-1 so last processor
            MPI_Recv(&new_sum, 1, MPI_INT, world_rank-1, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE); // receive from previous
            new_sum = new_sum+sum;  // update sum
            MPI_Send(&new_sum, 1, MPI_INT, 0, 1, MPI_COMM_WORLD);  // send the master process
        }else{ // if we are in between processors
            MPI_Recv(&new_sum, 1, MPI_INT, world_rank-1, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE); // receive from previous
            new_sum = new_sum+sum; // update sum
            MPI_Send(&new_sum, 1, MPI_INT, world_rank+1, 1, MPI_COMM_WORLD); // send sum to next
        }
        cout<<"Sum of Armstrong Numbers in Process "<<world_rank<<" = " <<sum<<endl;
    }
    MPI_Finalize(); // finalize parallel processing
}
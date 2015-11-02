#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/time.h>


#define Max 10000

FILE *matrix_a;
FILE *matrix_b;
FILE *matrix_c1;
FILE *matrix_c2;

char *dimension[Max];
char *final_dim[Max];
int matA[Max][Max];
int matB[Max][Max];
int matC[Max][Max];

struct v
{
    int r; //#row in A
    int c; //#col in B
    int num_x;
    int num_y;
    int num_z;
};


void read_dim_B(FILE *f);
void matrix_mult();
void read_dim_A(FILE *f);
void parse_first(char temp[]);
void *multByRow(void *dim);
void *multByElement(void *dim);
void writing_output1(int x , int z);
void writing_output2(int x , int z);

int main(int argc , char *argv[])
{




    if(argc == 1)
    {
        //argv[1]="0";
        // printf("after");



        matrix_a = fopen("a.txt" , "r");
        matrix_b = fopen("b.txt" , "r");
        matrix_c1 = fopen("c_1.txt" , "w");
        matrix_c2 = fopen("c_2.txt" , "w");

        matrix_mult();

        fclose(matrix_a);
        fclose(matrix_b);
        fclose(matrix_c1);
        fclose(matrix_c2);
    }
    else
    {
        matrix_a = fopen(argv[1] , "r");
        matrix_b = fopen(argv[2] , "r");
        matrix_c1 = fopen( strcat(argv[3] , "_1"), "w");
        matrix_c2 = fopen( strcat(argv[3] , "_2"), "w");

        matrix_mult();

        fclose(matrix_a);
        fclose(matrix_b);
        fclose(matrix_c1);
        fclose(matrix_c2);
    }






    return 0;
}

void parse_first(char temp[])  /**parsing the first line in the file*/
{
    int count =0;
    char * pch;
    pch = strtok (temp," \t\n");
    while (pch != NULL)
    {
        if(dimension[count]==NULL)
        {
            dimension[count]=(char*)malloc(sizeof(char)*strlen(pch)+1);
        }
        else
        {
            bzero(dimension[count], strlen(dimension[count]));
        }
        strncat(dimension[count],pch,strlen(pch));
        count++;
        pch = strtok (NULL," \t\n");

    }

    ////////////////////////////////////////////

    int i=0;
    int end= count ;

    count = 0;
    for(i=0 ; i < end ; i++)
    {
        temp = dimension[i];
        pch = strtok (temp,"=");
        while (pch != NULL)
        {
            if(final_dim[count]==NULL)
            {
                final_dim[count]=(char*)malloc(sizeof(char)*strlen(pch)+1);
            }
            else
            {
                bzero(final_dim[count], strlen(final_dim[count]));
            }
            strncat(final_dim[count],pch,strlen(pch));
            count++;
            pch = strtok (NULL,"=");

        }

    }

}


void read_dim_A(FILE *fp)  /**for reading the number of rows and columns from the file*/
{
    char buff[Max];
    fgets(buff, sizeof buff , (FILE*)fp);
    parse_first(buff);
    int x = atoi(final_dim[1]);
    int y = atoi(final_dim[3]);
    int i , j;
    for(i=0 ; i < x ; i++)
    {
        for(j=0 ; j < y ; j++)
        {
            fscanf(fp , "%d" , &matA[i][j]);
        }
    }

}

void read_dim_B(FILE *fp)  /**for reading the number of rows and columns from the file*/
{
    char buff[Max];
    fgets(buff, sizeof buff , (FILE*)fp);
    parse_first(buff);
    int x = atoi(final_dim[1]);
    int y = atoi(final_dim[3]);
    int i , j;
    for(i=0 ; i < x ; i++)
    {
        for(j=0 ; j < y ; j++)
        {
            fscanf(fp , "%d" , &matB[i][j]);
        }
    }

}


void *multByRow(void *dim)
{
    struct v *dim2;
    dim2 = (struct v*) dim;
    int i=dim2->r;
    int y = dim2->num_y;
    int z = dim2->num_z;
    int t , k;
    for(t=0 ; t < z ; t++)
    {
        for(k=0 ; k < y ; k++)
        {
            matC[i][t]+= matA[i][k]*matB[k][t];
        }
    }
    pthread_exit(NULL);

}

void *multByElement(void *dim)
{
    struct v *dim2;
    dim2 = (struct v*) dim;
    int y = dim2->num_y;
    int t;
    for(t=0 ; t < y ; t++)
    {
        matC[dim2->r][dim2->c]  +=  (matA[dim2->r][t] * matB[t][dim2->c]);
    }
    pthread_exit(NULL);
}

void writing_output1(int x , int z)
{
    int i ,j;
    for(i=0 ; i< x ; i++)
    {
        for( j=0 ; j < z ; j++)
        {
            fprintf(matrix_c1 , "%d\t" , matC[i][j]);
        }
        fprintf(matrix_c1 , "\n");
    }
}

void writing_output2(int x , int z)
{
    int i ,j;
    for(i=0 ; i< x ; i++)
    {
        for( j=0 ; j < z ; j++)
        {
            fprintf(matrix_c2 , "%d\t" , matC[i][j]);
        }
        fprintf(matrix_c2 , "\n");
    }
}
void matrix_mult() /**General Function*/
{
    int x , y ,z ;
    read_dim_A(matrix_a);
    x = atoi(final_dim[1]);
    y = atoi(final_dim[3]);
    read_dim_B(matrix_b);
    z = atoi(final_dim[3]);


//////////////////////////////////////////////////////////




    /**multiply by row*/

    printf("*****Multiply by Row*****\n");
    struct timeval stop, start;

    gettimeofday(&start, NULL); //start checking time
    int numOfThreads=0;
    pthread_t threads[x];
    int t , rc;
    for(t=0 ; t < x ; t++)
    {
        struct v *dim = (struct v *)malloc(sizeof(struct v));
        dim->num_x = x;
        dim->num_y = y;
        dim->num_z = z;
        dim->r = t;
        rc = pthread_create(&threads[t] , NULL , multByRow , (void*)dim);
        if(rc)
        {
            printf("ERROR: return code from pthread_create() is %d\n", rc);
            exit(-1);
        }
    }

    for(t=0 ; t < x ; t++)
    {
        pthread_join(threads[t] , NULL);
        numOfThreads++;
    }

    /**< Printing the number of threads in the stdout */
    printf("The Number Of Threads = %d\n" , numOfThreads);
    gettimeofday(&stop, NULL); //end checking time
    printf("Seconds taken %lu\n", stop.tv_sec - start.tv_sec);
    printf("Microseconds taken: %lu\n", stop.tv_usec - start.tv_usec);

    writing_output1(x,z);


    /**multiply by element*/

    printf("*****Multiply by Element*****\n");
    gettimeofday(&start, NULL); //start checking time
    pthread_t threads1[x][z];


    int row , col;
    int numOfThreads1=0;
    int rc1;
    for(row=0 ; row < x ; row++)
    {
        for(col=0 ; col < z ; col++)
        {

            struct v *dim2 = (struct v *)malloc(sizeof(struct v));
            dim2->num_x = x;
            dim2->num_y = y;
            dim2->num_z = z;

            dim2->r = row;
            dim2->c = col;
            rc1 = pthread_create(&threads1[row][col] , NULL , multByElement , (void*)dim2);
            if(rc1)
            {
                printf("ERROR: return code from pthread_create() is %d\n", rc1);
                exit(-1);
            }


        }

    }

    int k,q;/**> waiting until all the

                threads are finished

                */
    for(k=0 ; k < x ; k++)
    {
        for(q=0 ; q < z ; q++)
        {
            pthread_join(threads1[k][q] , NULL);
            numOfThreads1++;
        }

    }

    /**< Printing the number of threads in the stdout */
    printf("The Number Of Threads = %d\n" , numOfThreads1);

    gettimeofday(&stop, NULL); //end checking time
    printf("Seconds taken %lu\n", stop.tv_sec - start.tv_sec);
    printf("Microseconds taken: %lu\n", stop.tv_usec - start.tv_usec);


    writing_output2(x,z);




}

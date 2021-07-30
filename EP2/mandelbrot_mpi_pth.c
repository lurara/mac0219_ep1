#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <pthread.h>
#include "mpi.h"
#include <time.h>
#include <sys/time.h>

// #define _POSIX_C_SOURCE 199309L

#define MAX 100
#define TAG 19

#define MIN(a,b) (a < b) ? a : b

double c_x_min;
double c_x_max;
double c_y_min;
double c_y_max;

double pixel_width;
double pixel_height;

int iteration_max = 200;

int image_size;
unsigned char **image_buffer;

int i_x_max;
int i_y_ini;
int i_y_max;
int image_buffer_size;
int worker_size;

int meu_rank;
int np;

int i_start_y;
int i_end_y;

int gradient_size = 16;
int colors[17][3] = {
                        {66, 30, 15},
                        {25, 7, 26},
                        {9, 1, 47},
                        {4, 4, 73},
                        {0, 7, 100},
                        {12, 44, 138},
                        {24, 82, 177},
                        {57, 125, 209},
                        {134, 181, 229},
                        {211, 236, 248},
                        {241, 233, 191},
                        {248, 201, 95},
                        {255, 170, 0},
                        {204, 128, 0},
                        {153, 87, 0},
                        {106, 52, 3},
                        {16, 16, 16},
                    };

int n_threads;
pthread_t threads[MAX];
pthread_mutex_t mutex; // mutex para computação das iterações

int* iterations;
int * all_iterations;

typedef struct {

    int y_ini;
    int y_max;
    int p_idx;

}Args;

void allocate_image_buffer () {

    int rgb_size = 3;

    if(meu_rank == 0) {

        image_buffer = (unsigned char **) malloc(sizeof(unsigned char *) * image_buffer_size);

        for (int i = 0; i < image_buffer_size; i++){
            image_buffer[i] = (unsigned char *) malloc(sizeof(unsigned char) * rgb_size);
        }
        
        all_iterations = (int *) malloc(sizeof (int) * image_buffer_size);
    }

    //iterations = (int *) malloc (sizeof (int ) * image_buffer_size);
    iterations = (int *) malloc (sizeof (int) * worker_size);
};

void init(int argc, char *argv[]){

    if(argc < 2){

        if(meu_rank == 0) {

        printf("usage: ./mandelbrot_pth <n_threads>\n");
        printf("usage: ./mandelbrot_pth c_x_min c_x_max c_y_min c_y_max image_size\n");
        printf("examples with image_size = 11500:\n");
        printf("    Full Picture:         ./mandelbrot_pth -2.5 1.5 -2.0 2.0 11500 <n_threads>\n");
        printf("    Seahorse Valley:      ./mandelbrot_pth -0.8 -0.7 0.05 0.15 11500 <n_threads>\n");
        printf("    Elephant Valley:      ./mandelbrot_pth 0.175 0.375 -0.1 0.1 11500 <n_threads>\n");
        printf("    Triple Spiral Valley: ./mandelbrot_pth -0.188 -0.012 0.554 0.754 11500 <n_threads>\n");
        }

        MPI_Finalize();
        
        exit(0);
    }
    else{

        //c_x_min = -0.188;
        //c_x_max = -0.012;
        //c_y_min = 0.554;
        //c_y_max = 0.754;
        //image_size = 4096;

        //sscanf(argv[1], "%d", &n_threads);

        sscanf(argv[1], "%lf", &c_x_min);
        sscanf(argv[2], "%lf", &c_x_max);
        sscanf(argv[3], "%lf", &c_y_min);
        sscanf(argv[4], "%lf", &c_y_max);
        sscanf(argv[5], "%d", &image_size);
        sscanf(argv[6], "%d", &n_threads);

        i_x_max           = image_size;
        i_y_max           = image_size;

        pixel_width       = (c_x_max - c_x_min) / i_x_max;
        pixel_height      = (c_y_max - c_y_min) / i_y_max;

        image_buffer_size = image_size * image_size;  // added

        i_end_y = (meu_rank+1) * image_size/np;
        i_start_y = (meu_rank) * image_size/np;

        worker_size = image_buffer_size/np;
        // printf("Processo %d, de %d a %d\n", meu_rank, i_start_y, i_end_y);


        pthread_mutex_init(&mutex, NULL);
        
    };
};

void update_rgb_buffer(int iteration, int x, int y){

    // printf ("%d entrou do update\n", meu_rank);

    int color;

    if(iteration == iteration_max){
        image_buffer[(i_y_max * y) + x][0] = colors[gradient_size][0];
        image_buffer[(i_y_max * y) + x][1] = colors[gradient_size][1];
        image_buffer[(i_y_max * y) + x][2] = colors[gradient_size][2];
    }
    else{
        color = iteration % gradient_size;

        image_buffer[(i_y_max * y) + x][0] = colors[color][0];
        image_buffer[(i_y_max * y) + x][1] = colors[color][1];
        image_buffer[(i_y_max * y) + x][2] = colors[color][2];
    };

    // printf ("%d saiu do update\n", meu_rank);

};

void write_to_file(){
    FILE * file;
    char * filename               = "output.ppm";
    char * comment                = "# ";

    int max_color_component_value = 255;

    file = fopen(filename,"wb");

    fprintf(file, "P6\n %s\n %d\n %d\n %d\n", comment,
            i_x_max, i_y_max, max_color_component_value);

    for(int i = 0; i < image_buffer_size; i++){
        fwrite(image_buffer[i], 1 , 3, file);
    };

    fclose(file);
};

void* part_compute (void* args) {

    Args * arg = args;
    int y_ini = arg->y_ini;
    int y_max = arg->y_max;
    int idx = arg->p_idx;

    // printf("%d no part compute -> de %d a %d\n", meu_rank, y_ini, y_max);

    double z_x;
    double z_y;
    double z_x_squared;
    double z_y_squared;
    double escape_radius_squared = 4;

    int iteration;
    int i_x;
    int i_y;

    double c_x;
    double c_y;

    //int count = 0;
    int count = idx;

    for(i_y = y_ini; i_y < y_max; i_y++) {

        c_y = c_y_min + i_y * pixel_height;

        if (fabs(c_y) < pixel_height / 2)
            c_y = 0.0;
        
        for (i_x = 0; i_x < i_x_max; i_x++) {

            c_x = c_x_min + i_x * pixel_width;

            z_x = 0.0;
            z_y = 0.0;

            z_x_squared = 0.0;
            z_y_squared = 0.0;

            for(iteration = 0;
                iteration < iteration_max && \
                ((z_x_squared + z_y_squared) < escape_radius_squared);
                iteration++){
                z_y= 2 * z_x * z_y + c_y;
                z_x= z_x_squared - z_y_squared + c_x;

                z_x_squared = z_x * z_x;
                z_y_squared = z_y * z_y;
            }

            pthread_mutex_lock(&mutex);

            //if(i_x == i_x_max-1 && i_y == y_max-1 || i_y == 3211 && i_x == 2320)
            //printf("thread: count.. %d e iteration... %d\n", count, iteration);

            iterations[count++] = iteration;
            pthread_mutex_unlock(&mutex);

        }

    }

    // printf("%d saiu do part compute -> de %d a %d, com tam %d\n", meu_rank, y_ini, y_max,count);

    pthread_exit(0);

}

void compute_mandelbrot() {

    int size = (i_end_y - i_start_y)/n_threads;
    
    // printf ("%d no compute geral: %d\n", meu_rank, size);
    //Args* args = malloc(sizeof(Args));
	//t_struct * str = malloc(sizeof(t_struct)*threads);
    Args * args = malloc(sizeof(Args)*n_threads);

    for(int i = 0; i < n_threads; i++) {
        args[i].y_ini = i_start_y + i*size;
        args[i].y_max = ((i_start_y + (i+1)*size) > i_end_y) ? i_end_y : (i_start_y + (i+1)*size);
        args[i].p_idx = i*(worker_size/n_threads);
    }

    for (int i = 0; i < n_threads; i++) {
        pthread_create(&threads[i], NULL, part_compute, (void*) &args[i]);
    }

    for (int i = 0; i < n_threads; i++)
        pthread_join(threads[i], NULL);

    // printf ("%d saiu do compute \n", meu_rank);

    free(args);

};

int main(int argc, char *argv[]){

    struct timeval t_ini, t_end;

    // Iniciando o MPI:
    MPI_Init(&argc, &argv);

    /* MPI_Comm_rank retorna o rank de um processo no seu segundo argumento */
    MPI_Comm_rank(MPI_COMM_WORLD, &meu_rank);

    /* MPI_Comm_size: retorna o número de processos em um comunicador no seu segundo argumento */
    MPI_Comm_size(MPI_COMM_WORLD,&np);

    if (meu_rank == 0) {
        gettimeofday(&t_ini, NULL);
    }

    init(argc, argv);

    allocate_image_buffer();

    // printf ("%d antes do compute\n", meu_rank);
    //printf ("%d -> de %d a %d \n", meu_rank, i_y_ini, i_y_max);

    //if (meu_rank != 0)
    compute_mandelbrot();

    /* Cada processo, incluindo o raiz, 
     * manda uma mensagem para o processo raiz, que ao 
     * recebê-la, armazena-as na ordem de chegada. */

    // printf ("%d chegou no gather\n", meu_rank);
    
    /*recv e senv?*/

//    MPI_Gather(image_buffer, image_buffer_size, MPI_INT, image_buffer, image_buffer_size/(np - 1), MPI_INT, 0, MPI_COMM_WORLD);

    MPI_Gather(iterations, image_buffer_size/np, MPI_INT, all_iterations, image_buffer_size/np, MPI_INT, 0, MPI_COMM_WORLD);

    // printf ("%d passou do gather\n", meu_rank);

    MPI_Barrier(MPI_COMM_WORLD);

    // se for root
    if (meu_rank == 0) {

        for (int y = 0, count = 0; y < i_y_max; y++)
            for (int x = 0; x < i_x_max; x++){
                update_rgb_buffer(all_iterations[count++], x, y);
            }

        write_to_file();

    }
    
    MPI_Finalize();

    if (meu_rank == 0) {
        gettimeofday(&t_end, NULL);
        float res = t_end.tv_sec - t_ini.tv_sec + (float) (t_end.tv_usec - t_ini.tv_usec)/1000000;
        fprintf (stdout, "%.5f", res);
    }



    return 0;
};


// update_rgb_buffer(iteration, i_x, i_y);
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <pthread.h>
#include <mpi.h>

#define MAX 100
#define TAG 19

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
int i_y_max;
int image_buffer_size;

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

typedef struct {

    int y_ini;
    int y_max;
    unsigned char ** buffer;

} Args;

void allocate_image_buffer(){
    int rgb_size = 3;
    image_buffer = (unsigned char **) malloc(sizeof(unsigned char *) * image_buffer_size);

    for(int i = 0; i < image_buffer_size; i++){
        image_buffer[i] = (unsigned char *) malloc(sizeof(unsigned char) * rgb_size);
    };
};

unsigned char ** allocate_buffer(unsigned char ** buffer){
    int rgb_size = 3;
    buffer = (unsigned char **) malloc(sizeof(unsigned char *) * image_buffer_size);

    for(int i = 0; i < image_buffer_size; i++){
        buffer[i] = (unsigned char *) malloc(sizeof(unsigned char) * rgb_size);
    };

    return buffer;
}

void init(int argc, char *argv[]){
    if(argc < 7){
        printf("usage: ./mandelbrot_pth c_x_min c_x_max c_y_min c_y_max image_size\n");
        printf("examples with image_size = 11500:\n");
        printf("    Full Picture:         ./mandelbrot_pth -2.5 1.5 -2.0 2.0 11500 <n_threads>\n");
        printf("    Seahorse Valley:      ./mandelbrot_pth -0.8 -0.7 0.05 0.15 11500 <n_threads>\n");
        printf("    Elephant Valley:      ./mandelbrot_pth 0.175 0.375 -0.1 0.1 11500 <n_threads>\n");
        printf("    Triple Spiral Valley: ./mandelbrot_pth -0.188 -0.012 0.554 0.754 11500 <n_threads>\n");
        exit(0);
        n_threads = 3;
    }
    else{
        sscanf(argv[1], "%lf", &c_x_min);
        sscanf(argv[2], "%lf", &c_x_max);
        sscanf(argv[3], "%lf", &c_y_min);
        sscanf(argv[4], "%lf", &c_y_max);
        sscanf(argv[5], "%d", &image_size);
        sscanf(argv[6], "%d", &n_threads);

        i_x_max           = image_size;
        i_y_max           = image_size;
        image_buffer_size = image_size * image_size;

        pixel_width       = (c_x_max - c_x_min) / i_x_max;
        pixel_height      = (c_y_max - c_y_min) / i_y_max;
    };
};

void update_rgb_buffer(int iteration, int x, int y, unsigned char ** buf){
    int color;

    if(iteration == iteration_max){
        buf[(i_y_max * y) + x][0] = colors[gradient_size][0];
        buf[(i_y_max * y) + x][1] = colors[gradient_size][1];
        buf[(i_y_max * y) + x][2] = colors[gradient_size][2];
    }
    else{
        color = iteration % gradient_size;

        buf[(i_y_max * y) + x][0] = colors[color][0];
        buf[(i_y_max * y) + x][1] = colors[color][1];
        buf[(i_y_max * y) + x][2] = colors[color][2];
    }
}

void copy_aux(unsigned char ** image_buffer, unsigned char ** aux_image_buffer, int index, int size) {
    int end = (index+1)*size;

    for(int i = index*size; i < end; i++) 
        for(int j = 0; j < 3; j++) 
            image_buffer[i][j] = aux_image_buffer[i][j];
}

/*
void update_rgb_buffer(int iteration, int x, int y){
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
}*/

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

//void* part_compute (void* args) {
unsigned char ** part_compute (int y_ini, int y_max, unsigned char ** img_buf) {

    /*
    Args* arg = (Args*) args;
    int y_ini = arg->y_ini;
    int y_max = arg->y_max;
    unsigned char ** img_buf = arg->buffer;*/

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

            update_rgb_buffer(iteration, i_x, i_y, img_buf);

        }

    }

    return img_buf;

    /*
    arg->buffer = img_buf;

    pthread_exit(0);*/

}

void* thread_func (void* args) {
    Args* arg = (Args*) args;
    int y_ini = arg->y_ini;
    int y_max = arg->y_max;
    unsigned char ** img_buf = arg->buffer;
    img_buf = part_compute(y_ini, y_max, arg->buffer);
    arg->buffer = img_buf;
    //arg->buffer = img_buf;
    printf("cabou\n");
    pthread_exit(0);
    return 0;
}

/*
void compute_mandelbrot() {

    int size = i_y_max/n_threads;

    for (int i = 0; i < n_threads; i++) {

        Args* args = malloc(sizeof(Args));
        args->y_ini = i*size;
        args->y_max = ((i+1)*size > i_y_max) ? i_y_max : (i+1)*size;
        pthread_create(&threads[i], NULL, part_compute, (void*) args);

    }

    for (int i = 0; i < n_threads; i++)
        pthread_join(threads[i], NULL);

    // double z_x;
    // double z_y;
    // double z_x_squared;
    // double z_y_squared;
    // double escape_radius_squared = 4;

    // int iteration;
    // int i_x;
    // int i_y;

    // double c_x;
    // double c_y;

    // for(i_y = 0; i_y < i_y_max; i_y++){
    //     c_y = c_y_min + i_y * pixel_height;

    //     if(fabs(c_y) < pixel_height / 2){
    //         c_y = 0.0;
    //     };

    //     for(i_x = 0; i_x < i_x_max; i_x++){
    //         c_x         = c_x_min + i_x * pixel_width;

    //         z_x         = 0.0;
    //         z_y         = 0.0;

    //         z_x_squared = 0.0;
    //         z_y_squared = 0.0;

    //         for(iteration = 0;
    //             iteration < iteration_max && \
    //             ((z_x_squared + z_y_squared) < escape_radius_squared);
    //             iteration++){
    //             z_y         = 2 * z_x * z_y + c_y;
    //             z_x         = z_x_squared - z_y_squared + c_x;

    //             z_x_squared = z_x * z_x;
    //             z_y_squared = z_y * z_y;
    //         };

    //         update_rgb_buffer(iteration, i_x, i_y);
    //     };
    // };
};*/

int main(int argc, char *argv[]){

    int meu_rank, np;
    char msg[MAX];
    unsigned char ** aux_image_buffer;
    int i = 0;

    init(argc, argv);
    
    // Iniciando o MPI:
    MPI_Init(&argc, &argv);
    /* MPI_Comm_rank retorna o rank de um processo no seu segundo argumento */
    MPI_Comm_rank(MPI_COMM_WORLD, &meu_rank);
    /* MPI_Comm_size: retorna o número de processos em um comunicador no seu segundo argumento */
    MPI_Comm_size(MPI_COMM_WORLD,&np);

    MPI_Status status;
    // ALOCAR UM BUFFER DE IMAGEM AUXILIAR...

    aux_image_buffer = allocate_buffer(aux_image_buffer); // não mexer mias nessa porra

    /* EP2 */

    /* IDEIA: temos a main e os workers. A main é identificada pelo index 0, enquanto os 
     * workers possuem outros ranks. A main é responsável por enviar para os workers por
     * quais seções (e com quantas threads) cada worker será responsável por rodar. é 
     * inteiramente possível que algum worker receba menos threads que outro, mas isso
     * não deve impactar de forma muito péssima a execução do programa.
     */

    int total_sessions = (np-1)*n_threads; // quantidade de seções a se dividir  
    //int part = i_y_max/total_sessions; // part = tamanho que o conjunto de thread vai ter que processar
    int part = i_y_max/(np-1);

    if(meu_rank == 0) { // main
        // end inicial, size of array, type of array element, index, tag
        // tag: serve para distinguir mensagens diferentes 
        allocate_image_buffer();
        printf("Número de processos: %d\n", np);
        printf("total: %d, part: %d\n", i_y_max, part);
        /* CÁLCULO DA SEPARAÇÃO DOS DADOS... */

        // se 2 proc e 4 threads, consigo dividir o negócio em 8 seções
        // CALCULAR O QUANTO VAI FALTAR EMC ADA DEPOIS....
        int leftover = i_y_max%total_sessions; // o que sobrou, alguma thread vai ter que rodar né
        int index; // identifica index de processo
        // poise o leftover vai ser dessa main daqui kkkkkkkkkk

        /* preciso passar de onde até onde vai rodar */
        for(int i = 1; i < np; i++) {
            //MPI_Send(msg[0],strlen(msg)+1,MPI_CHAR,destino,tag,MPI_COMM_WORLD);
            MPI_Send(&i, 1, MPI_INT, i, TAG, MPI_COMM_WORLD);
        }

        // Recebe dados 
        for(int i = 1; i < np; i++) {
            MPI_Recv(&aux_image_buffer, image_size*3, MPI_UNSIGNED_CHAR, MPI_ANY_SOURCE, TAG, MPI_COMM_WORLD, &status);
            MPI_Recv(&index, 1, MPI_INT, status.MPI_SOURCE, TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE); //?
            /* recebe os pedaços do buffer auxiliar, precisa copiar as partes */
            printf("index: %d ", index);
            printf("voltou do processin\n");
            //copy_aux(image_buffer, aux_image_buffer, index, part);
        }
        
        // CALCULAR LEFTOVER AQUI NO FINAL...
        /*
        if(leftover) {
            part_compute(part*np, i_y_max, image_buffer);
        }*/
        
        //write_to_file();

    }
    else { // workers
        // message, size of array, type of array element, index, tag , ..., can be status ignore
        //MPI_Recv(msg[0], 100, MPI_CHAR, origem, tag, MPI_COMM_WORLD, &status);
        MPI_Recv(&i, 1, MPI_INT, 0, TAG, MPI_COMM_WORLD, &status);

        // Cada processo roda n_threads threads
        // iniciando de "i"
        int subpart = part/n_threads;
        int sobra = part%n_threads; // a última thread pode ter que rodar a mais...

        printf("chegou no processin\n");

        // vai rodar tamanho subpart
        for (int j = 0; j < n_threads; j++) {
            Args* args = malloc(sizeof(Args));
            args->y_ini = (i-1)*part+(j*subpart);
            args->y_max = (i-1)*part+(j+1)*subpart;
            args->buffer = aux_image_buffer;
            if(j == n_threads-1 && sobra) {
                args->y_max = i*part;
            }
            printf("thread %d, %d from %d to %d\n", j, i, args->y_ini, args->y_max);
            pthread_create(&threads[j], NULL, thread_func, (void*) args);
        }
    
        for (int j = 0; j < n_threads; j++)
            pthread_join(threads[j], NULL);

        printf("já vai mandar do processin\n");
        
        MPI_Send(&aux_image_buffer, image_size*3, MPI_UNSIGNED_CHAR, 0, TAG, MPI_COMM_WORLD);
        MPI_Send(&i, 1, MPI_INT, 0, TAG, MPI_COMM_WORLD);
        printf("enviou\n");
        

    }

    // DAR FREE NO BUFFERS AUXILIARES
    //free(aux_image_buffer);

    // Finalizando o MPI:
    MPI_Finalize();
    
    //write_to_file();

    return 0;
};

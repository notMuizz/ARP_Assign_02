#include "./../include/processA_utilities.h"
#include <bmpfile.h>
#include <sys/shm.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>
#include <signal.h>


#define WIDTH 1600
#define CENTERW WIDTH/2
#define HEIGHT 600
#define CENTERH HEIGHT/2
#define DEPTH 4
#define DIMFACTOR 20
#define RADIUS 20

#define SEM_PATH "/sem_image_1"

char *shm_name="/IMAGE";
int size=WIDTH*HEIGHT*sizeof(rgb_pixel_t);
int fd_shm;
rgb_pixel_t *ptr;

bmpfile_t *bmp;   

sem_t *sem1;

char *master_a="/tmp/master_a";
int fd_ma;

FILE* logfile;
time_t curtime;

void controller(int function,int line){

    time(&curtime);
    if(function==-1){
        fprintf(stderr,"Error: %s Line: %d\n",strerror(errno),line);
        fflush(stderr);
        fprintf(logfile,"TIME: %s Error: %s Line: %d\n",ctime(&curtime),strerror(errno),line);
        fflush(logfile);

        unlink(shm_name);
        munmap(ptr,size);
        bmp_destroy(bmp);
        sem_close(sem1);
        sem_unlink(SEM_PATH);

        fclose(logfile);
        exit(EXIT_FAILURE);
    }
}

void sa_function(int sig){

    if(sig==SIGTERM || sig==SIGINT){

        // closing all resources

        unlink(shm_name);
        munmap(ptr,size);
        bmp_destroy(bmp);
        sem_close(sem1);
        sem_unlink(SEM_PATH);

        fclose(logfile);

        exit(EXIT_SUCCESS);

    }
}

void bmp_circle(bmpfile_t *bmp,int w,int h){

    rgb_pixel_t p={0,255,0,0};

    for(int x = -RADIUS; x <= RADIUS; x++) {
        for(int y = -RADIUS; y <= RADIUS; y++) {
        // If distance is smaller, point is within the circle
            if(sqrt(x*x + y*y) < RADIUS) {
                /*
                * Color the pixel at the specified (x,y) position
                * with the given pixel values
                */
                bmp_set_pixel(bmp, w*DIMFACTOR + x, h*DIMFACTOR + y, p);  //multiply the coordinates by 20 to represent the circle on the bitmap
            }
        } 
    }
}

void delete(bmpfile_t *bmp){

    rgb_pixel_t pixel={0,0,255,0};   //BGRA

    for(int i=0;i<HEIGHT;i++)    
        for(int j=0;j<WIDTH;j++)           
            bmp_set_pixel(bmp,j,i,pixel);   //removing all pixels except for the red pixels
}

void static_conversion(bmpfile_t *bmp,rgb_pixel_t *ptr){
    
    rgb_pixel_t* p;

    for(int i=0;i<HEIGHT;i++)   
        for(int j=0;j<WIDTH;j++){   
            p=bmp_get_pixel(bmp,j,i);

            ptr[i+HEIGHT*j].alpha=p->alpha;      //passing the data from the bitmap to the shared memory treating it as an array
            ptr[i+HEIGHT*j].blue=p->blue;
            ptr[i+HEIGHT*j].green=p->green;
            ptr[i+HEIGHT*j].red=p->red;
        }
}

int main(int argc, char *argv[])
{   

    logfile=fopen("./logfiles/ProcessA_logfile","w");

    char *shm_name="/IMAGE";
    int size=WIDTH*HEIGHT*sizeof(rgb_pixel_t);
    int fd_shm;
    rgb_pixel_t *ptr;

    bmpfile_t *bmp;    

    pid_t a=getpid();


    struct sigaction sa;
    
    memset(&sa,0,sizeof(sa));
    sa.sa_flags=SA_RESTART;
    sa.sa_handler=&sa_function;
    controller(sigaction(SIGTERM,&sa,NULL),__LINE__);
    controller(sigaction(SIGINT,&sa,NULL),__LINE__);


    bmp = bmp_create(WIDTH, HEIGHT, DEPTH);   //creating bitmap
    if(bmp==NULL){
        exit(EXIT_FAILURE);
    }
        
    controller(fd_shm=shm_open(shm_name, O_CREAT|O_RDWR,0666),__LINE__);   //open the file descriptor of the shared memory

    controller(ftruncate(fd_shm,size),__LINE__);

    ptr=(rgb_pixel_t*)mmap(0,size,PROT_READ|PROT_WRITE,MAP_SHARED,fd_shm,0);
    if(ptr==MAP_FAILED){
        fprintf(stderr,"Error %s Line: %d\n",strerror(errno),__LINE__);
        exit(EXIT_FAILURE);
    }

    sem1=sem_open(SEM_PATH,O_CREAT,S_IRUSR | S_IWUSR,1);   // open the semaphore
    if(sem1==SEM_FAILED){
        fprintf(stderr,"Error: %s Line: %d\n",strerror(errno),__LINE__);
        fflush(stderr);
        unlink(shm_name);
        munmap(ptr,size);
        bmp_destroy(bmp);
        sem_unlink(SEM_PATH);
        exit(EXIT_FAILURE);
    }

    controller(sem_init(sem1,1,1),__LINE__);   //initialize the semaphore

    //opening pipe, writing and closing

    controller(fd_ma=open(master_a,O_WRONLY),__LINE__);

    controller(write(fd_ma,&a,sizeof(a)),__LINE__);

    close(fd_ma);

    //ending


    // Utility variable to avoid trigger resize event on launch
    int first_resize = TRUE;
    
    // Initialize UI
    init_console_ui();

    time(&curtime);
    delete(bmp); //clean the bitmap

    fprintf(logfile,"TIME: %s Cleaning of the bitmap\n",ctime(&curtime));
    fflush(logfile);

    bmp_circle(bmp,CENTERW/DIMFACTOR,CENTERH/DIMFACTOR);  //drawing a circle in the center of the image
    
    controller(sem_wait(sem1),__LINE__);  //wait sem

    static_conversion(bmp,ptr);    //passing data from the bitmap to the shared memory

    time(&curtime);
    fprintf(logfile,"TIME: %s Conversion for shared memory \n",ctime(&curtime));
    fflush(logfile);

    controller(sem_post(sem1),__LINE__);  //signal sem
 
    // Infinite loop
    while (TRUE)
    {
        // Get input in non-blocking mode
        int cmd = getch();

        // If user resizes screen, re-draw UI...
        if(cmd == KEY_RESIZE) {
            if(first_resize) {
                first_resize = FALSE;
            }
            else {
                reset_console_ui();
            }
        }

        // Else, if user presses print button...
        else if(cmd == KEY_MOUSE) {
            if(getmouse(&event) == OK) {
                if(check_button_pressed(print_btn, &event)) {
                    time(&curtime);
                    bmp_save(bmp,"./screenshot/imageA.bmp");
                    fprintf(logfile,"TIME: %s Screenshot of the bitmap\n",ctime(&curtime));
                    fflush(logfile);
                }
            }
        }

        // If input is an arrow key, move circle accordingly...
        else if(cmd == KEY_LEFT || cmd == KEY_RIGHT || cmd == KEY_UP || cmd == KEY_DOWN) {

            move_circle(cmd);

            draw_circle();

            delete(bmp); //clean the bitmap

            time(&curtime);
            fprintf(logfile,"TIME: %s Cleaning of the bitmap\n",ctime(&curtime));
            fflush(logfile);

            bmp_circle(bmp,circle.x,circle.y);  //drawing the circle in the image with the coordinates of the konsole circle

            controller(sem_wait(sem1),__LINE__);  //wait sem

            static_conversion(bmp,ptr);

            time(&curtime);
            fprintf(logfile,"TIME: %s Conversion for shared memory \n",ctime(&curtime));
            fflush(logfile);

            controller(sem_post(sem1),__LINE__);  //signal sem
        }
    }
    

    endwin();

    // closing, I will never arrive here, I wrote them for security
    
    controller(unlink(shm_name),__LINE__);
    controller(munmap(ptr,size),__LINE__);
    bmp_destroy(bmp);

    controller(sem_close(sem1),__LINE__);
    controller(sem_unlink(SEM_PATH),__LINE__);

    fclose(logfile);
    
    return 0;
}

/*
Egy vetélkedőben a játékvezető (szülő) két játékossal (2 gyerek) játszik.
A játékvezető mindig megvárja, a játék befejezését. 
A játékvezető színpadra hívja a játékosokat (létrehozza a gyerekeket) és megvárja, 
amíg felérnek és a játékra készségüket jelzik (jelzést küldenek szülőnek).
Mindketten bemutatkoznak (csövön átküldik a nevüket).
Nevet véletlenül választanak öt tetszőleges megadott névből. 
A játékvezető hangosan megismétel (kiírja a képernyőre) a kapott neveket!
A játékvezető készített 5 kérdést, hozzá megadta az öt jó választ (1-5 közti érték), 
majd feltesz ezek közül (véletlenszám) egy kérdést (csövön), amire egy számot, a helyes választ várja.
A kérdést mindkét játékos megkapja. A játékosok külön-külön válaszolnak csövön (véletlenszám). 
Mindkét játékos képernyőre írja a választ. 
A játékvezető (szülő) kiolvassa ezeket, majd szintén képernyőre írja.
Jó válasz esetén csokit, rossz válasz esetén semmit nem kap a játékos. 
A második fordulóban a kérdésre csak az első játékos válaszolhat, amiről a második játékos nem tud. 
A második játékos fogad rá látatlanban, hogy jó vagy rossz volt az első játékos válasza (üzenetsor).
A játékvezető kiolvassa és kiírja a választ és a fogadást. 
A jutalom ismét csoki vagy semmi, és a szülő kihirdeti (kiírja) az aktuális állást. 
A játékvezető végig vezeti a játékot maximum 5 kérdésig, aminek a végén nemcsak képernyőre írja a játék végeredményét, 
hanem jegyzőkönyvezi (fájlba is kiírja) is azt.

Készítette:
Mikolics Réka Szilvia
KD5XYP
*/
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/types.h>

#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>

//  Parent: reads from P_READ, writes on P_WRITE
//  Child 1.:  reads from C1_READ, writes on C1_WRITE
//  Child 2.:  reads from C2_READ, writes on C2_WRITE
#define P_READ    0
#define C_WRITE   1
#define C_READ    2
#define P_WRITE   3



// the total number of pipe *pairs* we need
#define NUM_PIPES   2


void sig_usr(int signo){
    if(signo == SIGINT)
        //printf("Signal caught from %d!\n");
    return;
}

int giveRandom(int lower, int upper) 
{ 
       
    int r = rand()% (upper + 1 - lower) + lower;      // Returns a pseudo-random integer between 0 and RAND_MAX.
    return r;
}

int giveExclusiveRandom(int lower, int upper, int asked[])
{   
    while(1)
    {
        bool equals = false;
        int r = rand()% (upper + 1 - lower) + lower;      // Returns a pseudo-random integer between 0 and RAND_MAX.
            for (int i = 0; i < 5; i++)
            {
                if (r == asked[i])
                {
                    equals = true;
                    break;
                }
                
            }
        if(!equals)
        {
            return r;
        }
    }

}


int main()
{

    //for pipes for children 1 :
    int fd[2*NUM_PIPES], nbytes;
    int val = 0, len, i;

    char string[] = "Hello, world!\n";
    char readbuffer1[80];

    //for pipes for children 2:
    int fd2[2*NUM_PIPES], nbytes2;
    int val2 = 0, len2, j;

    char string2[] = "Hello, world!\n";
    char readbuffer2[80];

    pid_t elso;
    pid_t masodik;

    int elsoEredmeny = 0;
    int masodikEredmeny = 0;
    int melyikJatekos = 0;


    char names[5][20] = {"Tom" , "Jerry" ,  "Kate", "Mary", "Jane"};

    char questions[5][50] = {"Hány lába van a kacsának?", "200 kcal kb. hány gombóc fagyi?", "Mennyi 5-(4+4)/2?","Hány évszak van egy hónapban?","Hány magos az az i7, amely 8 szálon fut?"};
    int answers[5]= {2,3,1,3,4};
    int askedQuestions[5] = {-1,-1,-1,-1,-1};
    char tippek[2][20] = {"nem találja el", "eltalálja"};

    // create all the descriptor pairs we need
    for (i=0; i<NUM_PIPES; ++i)
    {
        if (pipe(fd+(i*2)) < 0)
        {
            perror("Failed to allocate pipes");
            exit(EXIT_FAILURE);
        }
    }
    for (j=0; j<NUM_PIPES; ++j)
    {
        if (pipe(fd2+(j*2)) < 0)
        {
            perror("Failed to allocate pipes");
            exit(EXIT_FAILURE);
        }
    }

    if(elso = fork()) //parent
    {
        if(masodik = fork()) //parent
        {


        }
        else //child 2
        {
            unsigned int seed = (unsigned int)((unsigned) time(NULL) + getpid());
            srand(seed);
            //printf("Child2: %d  of parent %d\n" , getpid(), getppid());
            sleep(1);
            //printf("Signaling parent from 2. child..\n");
            printf("A 2.  játékos készen áll!..\n");
            kill(getppid(), SIGINT);

            signal(SIGINT,sig_usr);
            pause();

            // Child1. Start by closing descriptors we
            //  don't need in this process
            close(fd2[P_READ]);
            close(fd2[P_WRITE]);

            // used for output
            pid_t pid = getpid();

            int random2 = giveRandom(0,4);    
            /* Send NAME through the output side of pipe */
            write(fd2[C_WRITE], names[random2], (strlen(names[random2])+1));
            printf("A nevem: %s \n", names[random2]);
            char *myname2 = names[random2];

            for (int i = 1; i < 6; i++)
            {
                if (i%4 == 2)
                {
                    signal(SIGINT, sig_usr);
                    pause();
                    val2 = giveRandom(0,1);
                    write(fd2[C_WRITE], &val2, sizeof(val2));
                }
                else
                {
                    nbytes2 = read(fd2[C_READ], readbuffer2, sizeof(readbuffer2));
                    if(nbytes2 > 0)
                    {
                        val2 = giveRandom(1,5);
                        printf("(%s): A kérdésedre a válaszom  %d\n", myname2, val2);
                        write(fd2[C_WRITE], &val2, sizeof(val2));
                    }
                }  
            }
            return EXIT_SUCCESS;
        }
    }
    else //child 1
    {

        unsigned int seed = (unsigned int)((unsigned) time(NULL) + getpid());
        srand(seed);

        //printf("Child 1: %d  of parent %d \n" , getpid(), getppid());
        //printf("Signaling parent from 1. child\n");

        printf("Az 1. játékos készen áll!\n");
        kill(getppid(), SIGINT);

        signal(SIGINT,sig_usr);
        pause();


        // Child1. Start by closing descriptors we
        //  don't need in this process
        close(fd[P_READ]);
        close(fd[P_WRITE]);

        // used for output
        pid_t pid = getpid();

        int random1 = giveRandom(0,4);    
        /* Send NAME through the output side of pipe */
        write(fd[C_WRITE], names[random1], (strlen(names[random1])+1));
        printf("A nevem: %s \n", names[random1]);
        char *myname1 = names[random1];

        for (int i = 1; i < 6; i++)
        {
            if(i%4 == 0)
            {
                signal(SIGINT, sig_usr);
                pause();
                val = giveRandom(0,1);
                write(fd[C_WRITE], &val, sizeof(val));
            }
            else
            {
                nbytes = read(fd[C_READ], readbuffer1, sizeof(readbuffer1));
                if(nbytes > 0)
                {       
                        val = giveRandom(1,5);
                        printf("(%s): A kérdésedre a válszom: %d\n", myname1, val);
                        write(fd[C_WRITE], &val, sizeof(val));
                }
            }
            



            /*nbytes = read(fd[C_READ], readbuffer1, sizeof(readbuffer1));
            if(nbytes > 0)
            {       
                    val = giveRandom(1,5);
                    printf("(%s): I got your question(%s) and i think its %d\n", myname1, readbuffer1, val);
                    write(fd[C_WRITE], &val, sizeof(val));
            }*/
        }
        

    
        // finished. close remaining descriptors.
        close(fd[C_READ]);
        close(fd[C_WRITE]);


        return EXIT_SUCCESS;
    }       
        unsigned int seed = (unsigned int)((unsigned) time(NULL) + getpid());
        srand(seed);
            signal(SIGINT,sig_usr);
            pause();
            //printf("Signal recieved from 1. child! \n");
            printf("Tehát az első játékos megérkezett! \n");


            signal(SIGINT,sig_usr);
            pause();
            //printf("Signal recieved from 2. child");
            printf("Tehát a második játékos megérkezett! \n");

            //Send a singal back, to let them know we recieved their
            kill(elso, SIGINT);
            kill(masodik, SIGINT);

            // Parent. close unneeded descriptors
            close(fd[C_READ]);
            close(fd[C_WRITE]);
            close(fd2[C_READ]);
            close(fd2[C_WRITE]);

            pid_t pid = getpid();

            // READ THE NAMES
            nbytes = read(fd[P_READ], readbuffer1, sizeof(readbuffer1));
            char *elsonev = readbuffer1;
            printf("Üdv, %s ! \n", readbuffer1);
            nbytes2 = read(fd2[P_READ], readbuffer2, sizeof(readbuffer2));
            char* masodiknev = readbuffer2;
            printf("Üdv,  %s ! \n", readbuffer2);

            for (int i = 1; i < 6; i++)
            {
            
                if(i%2 == 0)
                {


                    //int randomq2 = giveRandom(0,4);

                    int randomq2 = giveExclusiveRandom(0,4, askedQuestions);
                    askedQuestions[i-1] = randomq2;
                    if(melyikJatekos%2 == 0)
                    {


                        //SENGING 2. QUESTION FOR PLAYER 1 - SECOND PLAYER IS GUESSING
                        write(fd[P_WRITE], questions[randomq2], (strlen(questions[randomq2])+1));
                        printf("A(z) %d . kérdésem %s -nek: %s \n",i,elsonev, questions[randomq2]);
                        printf("%s , te csak tippelhetsz, hogy %s eltalálja-e vagy sem. \n", masodiknev, elsonev);
                        kill(masodik, SIGINT);

                        //VÁLASZOK FELVÉTELE
                        read(fd[P_READ], &val, sizeof(val));
                        read(fd2[P_READ], &val2, sizeof(val2));

                    int helyesvalasz2 = answers[randomq2];
                    printf("A beérkezett válaszok: %s : %d, %s : %s, a helyes válasz pedig %d ! \n", elsonev, val, masodiknev, tippek[val2], helyesvalasz2);
                    if (val == helyesvalasz2)
                    {
                        elsoEredmeny++;
                        printf("Igen, %s, eltaláltad. Csoki!\n", elsonev);
                        if(val2 == 1)
                        {
                            masodikEredmeny++;
                            printf("Igen %s, jól tippeltél, %s eltalálta! \n", masodiknev, elsonev);
                        }
                        else
                        {
                            printf("%s, most nem talált, %s eltalálta! \n", masodiknev, elsonev);
                        }
                        
                    }
                    else
                    {
                        printf("Hát sajnos %s nem talált!", elsonev);
                        if(val2==0)
                        {
                            masodikEredmeny++;
                            printf("Igen %s, eltaláltad, mert  %s rosszul válaszolt! \n", masodiknev, elsonev);
                        }

                        
                    }
                    }
                    else
                    {


                        //SENGING 2. QUESTION FOR PLAYER 2 - FIRST PLAYER IS GUESSING
                        write(fd2[P_WRITE], questions[randomq2], (strlen(questions[randomq2])+1));
                        printf("A(z) %d . kérdésem %s -nek: %s \n",i,masodiknev, questions[randomq2]);
                        printf("%s most neked kell tippelni, hogy  %s eltalálja e vagy sem! \n", elsonev, masodiknev);
                        kill(elso, SIGINT);

                        //VÁLASZOK FELVÉTELE
                        read(fd[P_READ], &val, sizeof(val));
                        read(fd2[P_READ], &val2, sizeof(val2));

                    int helyesvalasz2 = answers[randomq2];
                    printf("A beérkezett válaszok: %s : %s, %s : %d, a helyes válasz pedig %d ! \n", elsonev, tippek[val], masodiknev, val2, helyesvalasz2);
                    if (val2 == helyesvalasz2)
                    {
                        masodikEredmeny++;

                        printf("Igen %s, eltaláltad. Csoki!\n", masodiknev);
                        if(val == 1)
                        {
                            elsoEredmeny++;
                            printf("Igen %s, neked is igazad lett, mert %s eltalálta! \n", elsonev, masodiknev);
                        }
                        else
                        {
                            printf("Nem %s, nem találtad el, mert %s eltalálta!\n", elsonev, masodiknev);
                        }
                        
                    }
                    else
                    {
                        printf("%s nem talált! \n", masodiknev);
                        if(val==0)
                        {
                            elsoEredmeny++;
                            printf("Igen %s, eltaláltad ,  %s rosszul válaszolt!\n", elsonev, masodiknev);
                        }
                    }
                    }

                    
                    melyikJatekos++; //két körönként melyik játékos kapja a kérdést!
                }
                else
                {
                    //SENGING 1. QUESTION
                    int randomq = giveExclusiveRandom(0,4,askedQuestions);
                    askedQuestions[i-1] = randomq;
                    write(fd[P_WRITE], questions[randomq], (strlen(questions[randomq])+1));
                    write(fd2[P_WRITE], questions[randomq], (strlen(questions[randomq])+1));
                    printf("A(z) %d kérdésem mindkettőtöknek szól: %s \n", i, questions[randomq]);
                    
                    //VÁLASZOK FELVÉTELE
                    read(fd[P_READ], &val, sizeof(val));
                    read(fd2[P_READ], &val2, sizeof(val2));

                    int helyesvalasz = answers[randomq];

                    printf("A beérkezett válaszok: %s : %d, %s : %d, a helyes válasz pedig %d ! \n", elsonev, val, masodiknev, val2, helyesvalasz);
                    if(val == helyesvalasz)
                    {
                        elsoEredmeny++;
                        printf("%s, eltaláltad, kapsz egy csokit!\n", elsonev);
                    }
                    if(val2 == helyesvalasz)
                    {
                        masodikEredmeny++;
                        printf("%s, eltaláltad, kapsz egy csokit!\n", masodiknev);
                    }
                }

                
                printf("\n");
            }  
            // close down remaining descriptors
            close(fd2[P_READ]);
            close(fd2[P_WRITE]);

            printf("Az első játékos( %s ) %d pontot ért el, a második( %s ) pedig %d -t. \n",elsonev, elsoEredmeny ,masodiknev, masodikEredmeny);

               FILE *fp;

                fp = fopen("jatekeredmeny.txt", "a");
                fprintf(fp, "Az első játékos(%s) %d pontot ért el, a második(%s) pedig %d -t. \n",elsonev,elsoEredmeny, masodiknev, masodikEredmeny);
                fclose(fp);
            // wait for child termination
            wait(NULL);

            return EXIT_SUCCESS;
    
}

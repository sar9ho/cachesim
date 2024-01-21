#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <limits.h>




int memory[16777216]={0x00};    //2^24-1
int addressbase16ver;           //base16 version of address
int access_size;                //size of access from trace 
int valin16;                    //dup of addressbase16ver
char addressac[256];            //original address captured (string)
char strval[256];               //original address captures as string -- dup
char loadstore[24];             //load or store -- from trace

//FUNCTIONS ILL NEED ---------------------------------------------------------------

int log2(int n) {
    int r=0;
    while (n>>=1) r++;
    return r;
}

int ones(int n){
    return ((1 << n) - 1);
    }

int powerOfTwo(int n ){ 
    return 1 << n;}

int leftShift(int x, int n){
    return x << n;}

int rightShift(int x, int n){
    return x >> n;}

int getLowerN(int num, int n){
    return num & ones(n);
    }

int ridLowerN(int num, int n){
    return num >> n << n;
    }

//-----------------------------------------------------------------------------------

struct Block{

    bool valid;
    int rank;
    int tag;

};

//MAIN-------------------------------------------------------------------------------------


int main(int argc, char* argv[]){

    int cache_size;
    int associativity;
    int block_size;

    FILE* trace = fopen(argv[1], "r");

    cache_size = atoi(argv[2])*1024;
    associativity = atoi(argv[3]);
    block_size = atoi(argv[4]);
    
    int numframes = cache_size/block_size;
    int set = numframes/associativity;

    int offset_bits = log2(block_size);
    int index_bits = log2(set);
    int tag_bits = 24 - index_bits - offset_bits;
    int highest_rank = 0;


    struct Block cache[set][associativity];
    for(int v = 0; v < set; v++){
        for(int b = 0; b < associativity; b++){
            cache[v][b].rank = -1;
            cache[v][b].tag = -1;
            cache[v][b].valid = false;

        }
    }
    
    if(!trace){
        perror("ERROR OPENING FILE");
        return EXIT_FAILURE;
    }

    //operation type (load or store) = loadstore
    //24-bit address being accessed (CURRENTLY AS STRING) = addressac
    //size of the access in bytes = access_size

    while(fscanf(trace, "%s", loadstore)!= EOF){
        fscanf(trace, "%x", &addressbase16ver);
        fscanf(trace, "%d", &access_size);
        highest_rank++;


       // int index = lowerN(rightShift(addressAccessedInBase16, offsetBits),indexBits);
        int index = (rightShift(addressbase16ver, offset_bits)) & ones(index_bits);
        int tag = rightShift(addressbase16ver, (offset_bits+index_bits)) & ones(tag_bits);
        int block_offset = addressbase16ver & ones(offset_bits);
        //int tag = lowerN(rightShift(rightShift(addressAccessedInBase16, offsetBits), indexBits), tagBits);

        bool flag = false;                
        int z = 0;
        int load_on = strcmp(loadstore, "load");
        int store_on = strcmp(loadstore, "store");

            // if(cache[index][z].rank >= highest_rank){
            //     highest_rank = cache[index][z].rank;
            //     z++;
            // }

            if(strcmp(loadstore, "store") == 0){                 //0 = store
                fscanf(trace, "%s", strval); //basically addressac
                sscanf(strval, "%x", &valin16); //basically base16ver

                for(int i = 0; i < associativity; i++){
                    if(cache[index][i].tag == tag && cache[index][i].valid == true){

                        // highest_rank++;
                        printf("store 0x%x hit\n", addressbase16ver);
                        flag = true;
                        if(flag == true){
                            int ind = i;
                            for(int x = ind; x > 0; x--){
                                cache[index][i] = cache[index][x-1];
                            }
                            cache[index][0].tag = tag;

                        }

                        cache[index][0].rank = highest_rank; 

                        for (int i = 0; i < access_size; i++ ) sscanf(strval + 2 * i, "%2x", &memory[addressbase16ver + i]);  

                        printf("\n");    
                        break;
                        }


                    }


                if(!flag){
                    printf("store 0x%x miss\n", addressbase16ver);
                    for (int i = 0; i < access_size; i++){ 
                        sscanf(strval + 2 * i, "%2x", &memory[addressbase16ver + i]); 
                        printf("\n");            
                    }       
                }

            } 
            //-----------------------end of store
            else if(load_on== 0){

                flag = false;

                for(int i=0; i < associativity; i++){

                    if(cache[index][i].tag == tag && cache[index][i].valid ==true){

                        cache[index][i].rank = highest_rank;
                        printf("load 0x%x hit ", addressbase16ver);
                        flag = true;                        
                        if(flag == true){
                            int ind = i;
                            for(int x = ind; x > 0; x--){
                                cache[index][i] = cache[index][x-1];
                            }
                            cache[index][0].tag = tag;

                        }

                        cache[index][0].rank = highest_rank; 

                        for(int i = 0;i < access_size; i++){
                            printf("%02x", memory[addressbase16ver + i]);    
                        }
                        printf("\n");
                        break;
                    }
                }

                if(!flag){      

                    int lowrnk = INT_MAX;
                    int lowassoc = -1;
                    int i = 0;
                    // highest_rank++;

                    while(i < associativity){

                        if(cache[index][i].rank < lowrnk){

                            lowrnk = cache[index][i].rank;
                            lowassoc = i;

                            }

                        i++;

                        }

                    cache[index][lowassoc].tag = tag;
                    cache[index][lowassoc].valid = true;
                    cache[index][lowassoc].rank = highest_rank;

                    printf("load 0x%x miss ", addressbase16ver);

                    for(int i = 0;i < access_size; i++) printf("%02x", memory[addressbase16ver + i]);                       

                    printf("\n");

            }



        }

    }

    fclose(trace);

    return EXIT_SUCCESS;


}


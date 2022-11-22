//Author:Ganesh Mahendra Prabhu
//Email:ganesh.prabhu@okstate.edu
//Date: 11/15/2022
//Description: This file gets and returns the data based on refnum



#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include "readFile.h"


void getAllJewelfunc(char refnum[]){
    printf("refnum %s \n",refnum);
  	for(int i=0;i<47;i++){
            printf("Ref\tCategory\tTitle\tTags\tDescription\tPrice\n%s\t%s\t%s\t%s\t%s\t%d",catalogstr[i].ref,catalogstr[i].category,catalogstr[i].title,catalogstr[i].tags,catalogstr[i].description,catalogstr[i].price);
    }
}


int main(int argc, char *argv[]){

	if(readFile()==-1){
		printf("Error in Reading Cartier_catalog.txt\n");exit(1);
	}

      getAllJewelfunc();

  return 0;

}


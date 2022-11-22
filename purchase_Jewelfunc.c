//Author: Adil Aman Mohammed
//Email: adil.mohammed@okstate.edu
//Date: 11/15/2022
//Description: This file gets and returns the data based on refnum



#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include "readFile.h"


void purchaseJewelfunc(char refnum[]){
    printf("refnum %s \n",refnum);
  	for(int i=0;i<47;i++){
        if(strcmp(refnum,catalogstr[i].ref)==0){
            printf("##***CARTIER CATALOG JEWELERY STORE***###\nThis item %s has been returned by customer and add in list \nAmount Returned:%d\n",catalogstr[i].category,catalogstr[i].price);

            catalogstr[i].quantity=catalogstr[i].quantity-1;

        }
    }
}

int main(int argc, char *argv[]){

	if(readFile()==-1){
		printf("Error in Reading Cartier_catalog.txt\n");exit(1);
	}

      char refnum[20];
      strcpy(refnum, argv[1]);
      purchaseJewelfunc(refnum);

  return 0;

}


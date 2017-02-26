#include "pixutils.h"

//private methods -> make static
static pixMap* pixMap_init(unsigned char arrayType);
static pixMap* pixMap_copy(pixMap *p);


static pixMap* pixMap_init(unsigned char arrayType){
	//initialize everything to zero except arrayType
	pixMap *p;
	if(!(p=malloc(sizeof(pixMap)))) return 0;
	p->image=0;
	p->imageWidth=0;
	p->imageHeight=0;
	p->pixArray_arrays=0;
	p->pixArray_blocks = 0;
	p->pixArray_overlay = 0;
	p->arrayType = arrayType;
	return p;
}	

void pixMap_destroy (pixMap **p){
	//free all mallocs and put a zero pointer in *p
	pixMap *ptr = *p;
	if(ptr->pixArray_arrays) free(ptr->pixArray_arrays);
	if(ptr->pixArray_blocks) free(ptr->pixArray_blocks);
	if(ptr->pixArray_overlay) free(ptr->pixArray_overlay);

	if(ptr->image) free(ptr->image);

	if(*p) free(*p);
	*p = 0;
}
	
pixMap *pixMap_read(char *filename,unsigned char arrayType){
 	//library call reads in the image into p->image and sets the width and height
 	pixMap *p=pixMap_init(arrayType);

 	int error;
 	if((error=lodepng_decode32_file(&(p->image), &(p->imageWidth), &(p->imageHeight),filename))){
 		fprintf(stderr,"error %u: %s\n", error, lodepng_error_text(error));
		return 0;
	}

 	//allocate the 2-D rgba arrays
	if (arrayType == 0){
  		//can only allocate for the number of rows - each row will be an array of MAXWIDTH
  		//copy each row of the image into each row
  		p->pixArray_arrays = malloc(p->imageHeight*sizeof(rgba[MAXWIDTH]));
  		for (int i = 0; i < p->imageHeight; i++) {
	 		memcpy(p->pixArray_arrays[i], p->image + i*sizeof(rgba)*p->imageWidth, sizeof(rgba)*p->imageWidth);
  		}
	} else if (arrayType ==1){
		//allocate a block of memory (dynamic array of p->imageHeight) to store the pointers
		//use a loop allocate a block of memory for each row
 		//copy each row of the image into the newly allocated block

		p->pixArray_blocks = malloc(p->imageHeight*sizeof(rgba*));

		for( int i =0; i < p->imageHeight; i++) {
			p->pixArray_blocks[i] = malloc(p->imageWidth *sizeof(rgba));
			memcpy(p->pixArray_blocks[i], p->image + i*sizeof(rgba)*p->imageWidth, sizeof(rgba)*p->imageWidth);
		}
 	} else if (arrayType ==2){
	  	//allocate a block of memory (dynamic array of p->imageHeight) to store the pointers
	  	//set the first pointer to the start of p->image
	  	//each subsequent pointer is the previous pointer + p->imageWidth

	  	p->pixArray_overlay = malloc(p->imageHeight*sizeof(rgba*));
	  	p->pixArray_overlay[0]=(rgba*)p->image;
		for( int i =1; i < p->imageHeight; i++) {
			p->pixArray_overlay[i] = p->pixArray_overlay[i-1] + p->imageWidth;
		}

	} else {
		return 0;
	}	

	return p;
}

int pixMap_write(pixMap *p,char *filename){
	int error=0;

   //have to copy each row of the array into the corresponding row of the image	
	if (p->arrayType ==0){
		for (int i = 0; i < p->imageHeight; i++) {
			memcpy(p->image + i*sizeof(rgba)*p->imageWidth, p->pixArray_arrays[i], sizeof(rgba)*p->imageWidth);
		}
	} else if (p->arrayType ==1){
		for (int i = 0; i < p->imageHeight; i++) {
			memcpy(p->image + i*sizeof(rgba)*p->imageWidth, p->pixArray_blocks[i], sizeof(rgba)*p->imageWidth);		
		}
	}
	//library call to write the image out 
 	if(lodepng_encode32_file(filename, p->image, p->imageWidth, p->imageHeight)){
  		fprintf(stderr,"error %u: %s\n", error, lodepng_error_text(error));
  		return 1;
	}
	return 0;
}	 

int pixMap_rotate(pixMap *p,float theta){
	pixMap *oldPixMap=pixMap_copy(p);
	if(!oldPixMap)return 1;
	const float ox=p->imageWidth/2.0f;
	const float oy=p->imageHeight/2.0f;
	const float s=sin(degreesToRadians(-theta));
	const float c=cos(degreesToRadians(-theta));
	for(int y=0;y<p->imageHeight;y++){
		for(int x=0;x<p->imageWidth;x++){ 
			float rotx = c*(x-ox) - s * (oy-y) + ox;
   			float roty = -(s*(x-ox) + c * (oy-y) - oy);
	 		int rotj=rotx+.5;
	 		int roti=roty+.5; 
	 		if (roti >=0 && roti < oldPixMap->imageHeight && rotj >=0 && rotj < oldPixMap->imageWidth){
		 		if(p->arrayType==0) memcpy(p->pixArray_arrays[y]+x,oldPixMap->pixArray_arrays[roti]+rotj,sizeof(rgba));
		 		else if(p->arrayType==1) memcpy(p->pixArray_blocks[y]+x,oldPixMap->pixArray_blocks[roti]+rotj,sizeof(rgba));
		 		else if(p->arrayType==2) memcpy(p->pixArray_overlay[y]+x,oldPixMap->pixArray_overlay[roti]+rotj,sizeof(rgba));			 
			} else{
				if(p->arrayType==0) memset(p->pixArray_arrays[y]+x,0,sizeof(rgba));
		 		else if(p->arrayType==1) memset(p->pixArray_blocks[y]+x,0,sizeof(rgba));
		 		else if(p->arrayType==2) memset(p->pixArray_overlay[y]+x,0,sizeof(rgba));		
			}		
		}	
	}
	pixMap_destroy(&oldPixMap);
	return 0;
}

pixMap *pixMap_copy(pixMap *p){
	pixMap *new=pixMap_init(p->arrayType);
	//allocate memory for new image of the same size a p->image and copy the image
	new->image = malloc(p->imageHeight*p->imageWidth*sizeof(p->image));
	memcpy(new->image, p->image, sizeof(rgba)*p->imageHeight*p->imageWidth);
	new->imageHeight=p->imageHeight;
 	new->imageWidth=p->imageWidth;

	//allocate memory and copy the arrays. 

	if (new->arrayType ==0){
		new->pixArray_arrays = malloc(new->imageHeight*sizeof(rgba[MAXWIDTH]));
		memcpy(new->pixArray_arrays, p->pixArray_arrays, new->imageHeight * sizeof(rgba[MAXWIDTH]));  
	} else if (new->arrayType ==1){
		new->pixArray_blocks = malloc(new->imageHeight*sizeof(rgba*));
		for( int i =0; i < new->imageHeight; i++) {
			new->pixArray_blocks[i] = malloc(new->imageWidth*sizeof(rgba));
			memcpy(new->pixArray_blocks[i], p->pixArray_blocks[i], new->imageWidth*sizeof(rgba)); 
		}
	} else if (new->arrayType ==2){
		new->pixArray_overlay = malloc(new->imageHeight*sizeof(rgba*));
	 	new->pixArray_overlay[0]=(rgba*)new->image;
		for( int i =1; i < new->imageHeight; i++) {
			new->pixArray_overlay[i] = new->pixArray_overlay[i-1] + new->imageWidth;
		}
	}
	return new;
}	

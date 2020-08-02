/** @file TextureUtils.hpp
  * @brief Helper functions to work with OpenGL Textures
	* @author Dr. Jeffrey Paone
	* @date Last Edit: 21 Sep 2017
	* @version 1.2
	*
	* @copyright MIT License Copyright (c) 2017 Dr. Jeffrey Paone
	*
	*	These functions, classes, and constants help minimize common
	*	code that needs to be written.
  */

#ifndef __CSCI441_TEXTUREUTILS_H__
#define __CSCI441_TEXTUREUTILS_H__

#ifdef __APPLE__
	#include <OpenGL/gl.h>
#else
	#include <GL/gl.h>
#endif

#include <stdio.h>

////////////////////////////////////////////////////////////////////////////////////

/** @namespace CSCI441
  * @brief CSCI441 Helper Functions for OpenGL
  */
namespace CSCI441 {
	/**	@brief loads a PPM into memory
		*
		*  This function reads an ASCII PPM, returning true if the function succeeds and
		*      false if it fails. If it succeeds, the variables imageWidth and
		*      imageHeight will hold the width and height of the read image, respectively.
		*
		*  It's not terribly robust.
		*
		*  Returns the image as an unsigned character array containing
		*      imageWidth*imageHeight*3 entries (for that many bytes of storage).
		*
		*  NOTE: this function expects imageData to be UNALLOCATED, and will allocate
		*      memory itself. If the function fails (returns false), imageData
		*      will be set to NULL and any allocated memory will be automatically deallocated.
		*
		*	@param[in] const char *filename	- filename of the image to load
		* @param[out] int &imageWidth			-	will contain the image width upon successful completion
		* @param[out] int &imageHeight		- will contain the image height upon successful completion
		* @param[out] unsigned char* &imageData - will contain the RGB data upon successful completion
		* @pre imageData is unallocated
		*/
	bool loadPPM( const char *filename, int &imageWidth, int &imageHeight, unsigned char* &imageData );
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
// Internal definitions

namespace CSCI441_INTERNAL {

}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
// Outward facing function implementations

inline bool CSCI441::loadPPM( const char *filename, int &imageWidth, int &imageHeight, unsigned char* &imageData ) {
    FILE *fp = fopen(filename, "r");
    int temp, maxValue;
    fscanf(fp, "P%d", &temp);
    if(temp != 3) {
        fprintf(stderr, "Error: PPM file is not of correct format! (Must be P3, is P%d.)\n", temp);
        fclose(fp);
        return false;
    }

    //got the file header right...
    fscanf(fp, "%d", &imageWidth);
    fscanf(fp, "%d", &imageHeight);
    fscanf(fp, "%d", &maxValue);

    //now that we know how big it is, allocate the buffer...
    imageData = new unsigned char[imageWidth*imageHeight*3];
    if(!imageData) {
        fprintf(stderr, "Error: couldn't allocate image memory. Dimensions: %d x %d.\n", imageWidth, imageHeight);
        fclose(fp);
        return false;
    }

    //and read the data in.
    for(int j = 0; j < imageHeight; j++) {
        for(int i = 0; i < imageWidth; i++) {
            int r, g, b;
            fscanf(fp, "%d", &r);
            fscanf(fp, "%d", &g);
            fscanf(fp, "%d", &b);

            imageData[(j*imageWidth+i)*3+0] = r;
            imageData[(j*imageWidth+i)*3+1] = g;
            imageData[(j*imageWidth+i)*3+2] = b;
        }
    }

    fclose(fp);
    return true;
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
// Internal function implementations



#endif // __CSCI441_TEXTUREUTILS_H__

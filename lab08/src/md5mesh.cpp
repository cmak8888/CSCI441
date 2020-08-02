/*
 * md5mesh.c -- md5mesh model loader + animation
 * last modification: aug. 14, 2007
 *
 * Doom3's md5mesh viewer with animation.  Mesh portion.
 * Dependences: md5model.h, md5anim.c.
 *
 * Copyright (c) 2005-2007 David HENRY
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
 * CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#include <GL/glew.h>

#include <SOIL/SOIL.h>

#include <fstream>
#include <string>
using namespace std;

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "lab08.h"
#include "MD5/md5model.h"

#include <glm/gtc/matrix_transform.hpp>

/* vertex array related stuff */
int max_verts = 0;
int max_tris = 0;

vec3_t *vertexArray = NULL;
GLuint *vertexIndices = NULL;

vec2_t *texelArray = NULL;

// TODO #09A: place global VAO/VBO desciptors here

GLuint vaods2;
GLuint vbods10;
GLuint vbods11;

/**
 * Basic quaternion operations.
 */
void quat_computeW(quat4_t q) {
	float t = 1.0f - (q[X] * q[X]) - (q[Y] * q[Y]) - (q[Z] * q[Z]);

	if (t < 0.0f)
		q[W] = 0.0f;
	else
		q[W] = -sqrt (t);
}

void quat_normalize(quat4_t q) {
	/* compute magnitude of the quaternion */
	float mag = sqrt ((q[X] * q[X]) + (q[Y] * q[Y])
			+ (q[Z] * q[Z]) + (q[W] * q[W]));

	/* check for bogus length, to protect against divide by zero */
	if (mag > 0.0f) {
		/* normalize it */
		float oneOverMag = 1.0f / mag;

		q[X] *= oneOverMag;
		q[Y] *= oneOverMag;
		q[Z] *= oneOverMag;
		q[W] *= oneOverMag;
	}
}

void quat_multQuat(const quat4_t qa, const quat4_t qb, quat4_t out) {
	out[W] = (qa[W] * qb[W]) - (qa[X] * qb[X]) - (qa[Y] * qb[Y]) - (qa[Z] * qb[Z]);
	out[X] = (qa[X] * qb[W]) + (qa[W] * qb[X]) + (qa[Y] * qb[Z]) - (qa[Z] * qb[Y]);
	out[Y] = (qa[Y] * qb[W]) + (qa[W] * qb[Y]) + (qa[Z] * qb[X]) - (qa[X] * qb[Z]);
	out[Z] = (qa[Z] * qb[W]) + (qa[W] * qb[Z]) + (qa[X] * qb[Y]) - (qa[Y] * qb[X]);
}

void quat_multVec(const quat4_t q, const vec3_t v, quat4_t out) {
	out[W] = - (q[X] * v[X]) - (q[Y] * v[Y]) - (q[Z] * v[Z]);
	out[X] =   (q[W] * v[X]) + (q[Y] * v[Z]) - (q[Z] * v[Y]);
	out[Y] =   (q[W] * v[Y]) + (q[Z] * v[X]) - (q[X] * v[Z]);
	out[Z] =   (q[W] * v[Z]) + (q[X] * v[Y]) - (q[Y] * v[X]);
}

void quat_rotatePoint(const quat4_t q, const vec3_t in, vec3_t out) {
	quat4_t tmp, inv, final;

	inv[X] = -q[X]; inv[Y] = -q[Y];
	inv[Z] = -q[Z]; inv[W] =  q[W];

    quat_normalize(inv);

    quat_multVec(q, in, tmp);
    quat_multQuat(tmp, inv, final);

	out[X] = final[X];
	out[Y] = final[Y];
	out[Z] = final[Z];
}

GLuint loadTexture( string filename ) {
	GLuint textureHandle = SOIL_load_OGL_texture( filename.c_str(),
																								SOIL_LOAD_AUTO,
																								SOIL_CREATE_NEW_ID,
																								SOIL_FLAG_MIPMAPS | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT );
	if( textureHandle != 0 ) {
		glBindTexture(GL_TEXTURE_2D, textureHandle);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		printf("[.md5mesh]: %s texture map read in with handle %d\n", filename.c_str(), textureHandle);
	} else {

	}
	return textureHandle;
}

/**
 * Load an MD5 model from file.
 */
int read_MD5_model(const char *filename, struct md5_model_t *mdl) {
	FILE *fp;
	char buff[512];
	int version;
	int curr_mesh = 0;
	int i;

	int totVert = 0;
	int totWeights = 0;
	int totTris = 0;

	double minX =  999999, minY =  999999, minZ =  999999;
	double maxX = -999999, maxY = -999999, maxZ = -999999;

	printf( "[.md5mesh]: about to read %s\n", filename );

	fp = fopen (filename, "rb");
	if (!fp) {
		fprintf (stderr, "[.md5mesh]: Error: couldn't open \"%s\"!\n", filename);
		return 0;
	}

	while (!feof (fp)) {
		/* Read whole line */
		fgets (buff, sizeof (buff), fp);

		if (sscanf (buff, " MD5Version %d", &version) == 1) {
			if (version != 10) {
				/* Bad version */
				fprintf (stderr, "[.md5mesh]: Error: bad model version\n");
				fclose (fp);
				return 0;
			}
		} else if (sscanf (buff, " numJoints %d", &mdl->num_joints) == 1) {
			if (mdl->num_joints > 0) {
				/* Allocate memory for base skeleton joints */
				mdl->baseSkel = (struct md5_joint_t *)
                		calloc (mdl->num_joints, sizeof (struct md5_joint_t));
			}
		} else if (sscanf (buff, " numMeshes %d", &mdl->num_meshes) == 1) {
			if (mdl->num_meshes > 0) {
				/* Allocate memory for meshes */
				mdl->meshes = (struct md5_mesh_t *)
                		calloc (mdl->num_meshes, sizeof (struct md5_mesh_t));
			}
		} else if (strncmp (buff, "joints {", 8) == 0) {
			/* Read each joint */
			for (i = 0; i < mdl->num_joints; ++i) {
				struct md5_joint_t *joint = &mdl->baseSkel[i];

				/* Read whole line */
				fgets (buff, sizeof (buff), fp);

				if (sscanf (buff, "%s %d ( %f %f %f ) ( %f %f %f )",
						joint->name, &joint->parent, &joint->pos[0],
						&joint->pos[1], &joint->pos[2], &joint->orient[0],
						&joint->orient[1], &joint->orient[2]) == 8) {
					/* Compute the w component */
                    quat_computeW(joint->orient);
				}
			}
		} else if (strncmp (buff, "mesh {", 6) == 0) {
			struct md5_mesh_t *mesh = &mdl->meshes[curr_mesh];
			int vert_index = 0;
			int tri_index = 0;
			int weight_index = 0;
			float fdata[4];
			int idata[3];

			while ((buff[0] != '}') && !feof (fp)) {
				/* Read whole line */
				fgets (buff, sizeof (buff), fp);

				if (strstr (buff, "shader ")) {
					int quote = 0, j = 0;

					/* Copy the shader name whithout the quote marks */
					for (i = 0; i < sizeof (buff) && (quote < 2); ++i) {
						if (buff[i] == '\"')
							quote++;

						if ((quote == 1) && (buff[i] != '\"')) {
							mesh->shader[j] = buff[i];
							j++;
						}
					}

					/* there was a shader name */
					if( j > 0 ) {
						string diffuseMapFN = string(mesh->shader) + ".tga";
						mesh->textures[0].texHandle = loadTexture( diffuseMapFN );
						if( mesh->textures[0].texHandle == 0 ) {
							diffuseMapFN = string(mesh->shader) + ".png";
							mesh->textures[0].texHandle = loadTexture( diffuseMapFN );
						}

						string specularMapFN = string(mesh->shader) + "_s.tga";
						mesh->textures[1].texHandle = loadTexture( specularMapFN );
						if( mesh->textures[1].texHandle == 0 ) {
							specularMapFN = string(mesh->shader) + "_s.png";
							mesh->textures[1].texHandle = loadTexture( specularMapFN );
						}

						string normalMapFN = string(mesh->shader) + "_local.tga";
						mesh->textures[2].texHandle = loadTexture( normalMapFN );
						if( mesh->textures[2].texHandle == 0 ) {
							normalMapFN = string(mesh->shader) + "_local.png";
							mesh->textures[2].texHandle = loadTexture( normalMapFN );
						}

						string heightMapFN = string(mesh->shader) + "_h.tga";
						mesh->textures[3].texHandle = loadTexture( heightMapFN );
						if( mesh->textures[3].texHandle == 0 ) {
							heightMapFN = string(mesh->shader) + "_h.png";
							mesh->textures[3].texHandle = loadTexture( heightMapFN );
						}
					}
				} else if (sscanf (buff, " numverts %d", &mesh->num_verts) == 1) {
					if (mesh->num_verts > 0) {
						/* Allocate memory for vertices */
						mesh->vertices = (struct md5_vertex_t *)
                        		malloc (sizeof (struct md5_vertex_t) * mesh->num_verts);
					}

					if (mesh->num_verts > max_verts)
						max_verts = mesh->num_verts;

					totVert += mesh->num_verts;
				} else if (sscanf (buff, " numtris %d", &mesh->num_tris) == 1) {
					if (mesh->num_tris > 0) {
						/* Allocate memory for triangles */
						mesh->triangles = (struct md5_triangle_t *)
                        		malloc (sizeof (struct md5_triangle_t) * mesh->num_tris);
					}

					if (mesh->num_tris > max_tris)
						max_tris = mesh->num_tris;

					totTris += mesh->num_tris;
				} else if (sscanf (buff, " numweights %d", &mesh->num_weights) == 1) {
					if (mesh->num_weights > 0) {
						/* Allocate memory for vertex weights */
						mesh->weights = (struct md5_weight_t *)
                        		malloc (sizeof (struct md5_weight_t) * mesh->num_weights);
					}

					totWeights += mesh->num_weights;
				} else if (sscanf (buff, " vert %d ( %f %f ) %d %d", &vert_index,
						&fdata[0], &fdata[1], &idata[0], &idata[1]) == 5) {
					/* Copy vertex data */
					mesh->vertices[vert_index].st[0] = fdata[0];
					mesh->vertices[vert_index].st[1] = fdata[1];
					mesh->vertices[vert_index].start = idata[0];
					mesh->vertices[vert_index].count = idata[1];
				} else if (sscanf (buff, " tri %d %d %d %d", &tri_index,
						&idata[0], &idata[1], &idata[2]) == 4) {
					/* Copy triangle data */
					mesh->triangles[tri_index ].index[0] = idata[0];
					mesh->triangles[tri_index ].index[1] = idata[1];
					mesh->triangles[tri_index ].index[2] = idata[2];
				} else if (sscanf (buff, " weight %d %d %f ( %f %f %f )",
						&weight_index, &idata[0], &fdata[3],
						&fdata[0], &fdata[1], &fdata[2]) == 6) {
					/* Copy vertex data */
					mesh->weights[weight_index].joint  = idata[0];
					mesh->weights[weight_index].bias   = fdata[3];
					mesh->weights[weight_index].pos[0] = fdata[0];
					mesh->weights[weight_index].pos[1] = fdata[1];
					mesh->weights[weight_index].pos[2] = fdata[2];

					if( fdata[0] < minX ) { minX = fdata[0]; }
					if( fdata[0] > maxX ) { maxX = fdata[0]; }
					if( fdata[1] < minY ) { minY = fdata[1]; }
					if( fdata[1] > maxY ) { maxY = fdata[1]; }
					if( fdata[2] < minZ ) { minZ = fdata[2]; }
					if( fdata[2] > maxZ ) { maxZ = fdata[2]; }
				}
			}

			curr_mesh++;
		}
	}

	fclose (fp);

	printf( "[.md5mesh]: finished reading %s\n", filename );
	printf( "[.md5mesh]: read in %d meshes, %d joints, %d vertices, %d weights, and %d triangles\n", mdl->num_meshes, mdl->num_joints, totVert, totWeights, totTris );
	printf( "[.md5mesh]: base pose %f units across in X, %f units across in Y, %f units across in Z\n", (maxX - minX), (maxY-minY), (maxZ - minZ) );
	printf( "\n" );

	return 1;
}

/**
 * Free resources allocated for the model.
 */
void free_model(struct md5_model_t *mdl) {
	int i;

	if (mdl->baseSkel) {
		free (mdl->baseSkel);
		mdl->baseSkel = NULL;
	}

	if (mdl->meshes) {
		/* Free mesh data */
		for (i = 0; i < mdl->num_meshes; ++i) {
			if (mdl->meshes[i].vertices) {
				free (mdl->meshes[i].vertices);
				mdl->meshes[i].vertices = NULL;
			}

			if (mdl->meshes[i].triangles) {
				free (mdl->meshes[i].triangles);
				mdl->meshes[i].triangles = NULL;
			}

			if (mdl->meshes[i].weights) {
				free (mdl->meshes[i].weights);
				mdl->meshes[i].weights = NULL;
			}
		}

		free (mdl->meshes);
		mdl->meshes = NULL;
	}
}

/**
 * Prepare a mesh for drawing.  Compute mesh's final vertex positions
 * given a skeleton.  Put the vertices in vertex arrays.
 */
void prepare_mesh(const struct md5_mesh_t *mesh, const struct md5_joint_t *skeleton) {
	int i, j, k;

	/* Setup vertex indices */
	for (k = 0, i = 0; i < mesh->num_tris; ++i) {
		for (j = 0; j < 3; ++j, ++k)
			vertexIndices[k] = mesh->triangles[i].index[j];
	}

	/* Setup vertices */
	for (i = 0; i < mesh->num_verts; ++i) {
		vec3_t finalVertex = { 0.0f, 0.0f, 0.0f };

		/* Calculate final vertex to draw with weights */
		for (j = 0; j < mesh->vertices[i].count; ++j) {
			const struct md5_weight_t *weight = &mesh->weights[mesh->vertices[i].start + j];
			const struct md5_joint_t  *joint  = &skeleton[weight->joint];

			/* Calculate transformed vertex for this weight */
			vec3_t wv;
            quat_rotatePoint(joint->orient, weight->pos, wv);

			/* The sum of all weight->bias should be 1.0 */
			finalVertex[0] += (joint->pos[0] + wv[0]) * weight->bias;
			finalVertex[1] += (joint->pos[1] + wv[1]) * weight->bias;
			finalVertex[2] += (joint->pos[2] + wv[2]) * weight->bias;
		}

		vertexArray[i][0] = finalVertex[0];
		vertexArray[i][1] = finalVertex[1];
		vertexArray[i][2] = finalVertex[2];

		texelArray[i][0] = mesh->vertices[i].st[0];
		texelArray[i][1] = mesh->vertices[i].st[1];
	}
}

void draw_mesh(const struct md5_mesh_t *mesh) {
	/* Bind Diffuse Map */
	glBindTexture( GL_TEXTURE_2D, mesh->textures[0].texHandle );

	// TODO #10: bind VAO/VBO and update data
	glBindVertexArray(vaods2);

	glBindBuffer(GL_ARRAY_BUFFER, vbods10);
    glBufferSubData( GL_ARRAY_BUFFER, 0, 3 * sizeof( float ) * mesh->num_verts, vertexArray);

    glBufferSubData( GL_ARRAY_BUFFER, sizeof(float) * 3 * max_verts, 2 * sizeof(float) * mesh->num_verts, texelArray);

    glBindBuffer(GL_ARRAY_BUFFER, vbods11);
    glBufferSubData( GL_ELEMENT_ARRAY_BUFFER, 0, 3 * sizeof( GLuint ) * mesh->num_tris, vertexIndices);
	glBufferSubData( GL_ARRAY_BUFFER, 				sizeof(vec3_t) * max_verts, sizeof(vec2_t) * mesh->num_verts,    texelArray );


	Lab08BlackMagic::setModelMatrix();

	// TODO #11: draw our MD5 model
	glDrawElements(GL_TRIANGLES, mesh->num_tris*3, GL_UNSIGNED_INT, (void*)0);
}

void alloc_vertex_arrays() {
	vertexArray = (vec3_t *)malloc (sizeof (vec3_t) * max_verts);
	vertexIndices = (GLuint *)malloc (sizeof (GLuint) * max_tris * 3);

	texelArray = (vec2_t *)malloc (sizeof (vec2_t) * max_verts);

	// TODO #09: create VAO & VBO for MD5 Model
	glGenVertexArrays(1,&vaods2);
	glBindVertexArray(vaods2);

	glGenBuffers(1, &vbods10);
    glGenBuffers(1, &vbods11);

    glBindBuffer(GL_ARRAY_BUFFER, vbods10);
	glBufferData(GL_ARRAY_BUFFER, 5 * sizeof(float) * max_verts, NULL, GL_DYNAMIC_DRAW);

	glEnableVertexAttribArray(vertexPositionLocation);
    glVertexAttribPointer(vertexPositionLocation, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

    glEnableVertexAttribArray(vertexTexCoordLocation);
    glVertexAttribPointer(vertexTexCoordLocation, 2, GL_FLOAT, GL_FALSE, 0, (void*)(sizeof(float) * 3 * max_verts));

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbods11);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * max_tris * 3, NULL, GL_DYNAMIC_DRAW);

}

void free_vertex_arrays() {
	if (vertexArray) {
		free (vertexArray);
		vertexArray = NULL;
	}

	if (vertexIndices) {
		free (vertexIndices);
		vertexIndices = NULL;
	}

	if (texelArray) {
		free (texelArray);
		texelArray = NULL;
	}

	// TODO #12A: delete our MD5 buffers

    glDeleteBuffers(1, &vaods2);
    glDeleteBuffers(1, &vbods10);
    glDeleteBuffers(1, &vbods11);
}

/**
 * Draw the skeleton as lines and points (for joints).
 */
 void draw_skeleton(const struct md5_joint_t *skeleton, int num_joints) {
	 Lab08BlackMagic::drawSkeleton( skeleton, num_joints );
 }

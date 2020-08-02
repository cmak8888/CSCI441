/*
 * md5model.h -- md5mesh model loader + animation
 * last modification: aug. 14, 2007
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
 * gcc -Wall -ansi -lGL -lGLU -lglut md5anim.c md5anim.c -o md5model
 */

#ifndef __MD5MODEL_H__
#define __MD5MODEL_H__

/* Vectors */
typedef float vec2_t[2];
typedef float vec3_t[3];

/* Quaternion (x, y, z, w) */
typedef float quat4_t[4];

enum {
    X, Y, Z, W
};

/* Joint */
struct md5_joint_t
{
    char name[64];
    int parent;

    vec3_t pos;
    quat4_t orient;
};

/* Vertex */
struct md5_vertex_t
{
    vec2_t st;

    int start; /* start weight */
    int count; /* weight count */
};

/* Triangle */
struct md5_triangle_t
{
    int index[3];
};

/* Weight */
struct md5_weight_t
{
    int joint;
    float bias;

    vec3_t pos;
};

/* Texture Handles */
struct md5_texture_t
{
    int texHandle;
};

/* Bounding box */
struct md5_bbox_t
{
    vec3_t min;
    vec3_t max;
};

/* MD5 mesh */
struct md5_mesh_t
{
    struct md5_vertex_t *vertices;
    struct md5_triangle_t *triangles;
    struct md5_weight_t *weights;
    struct md5_texture_t textures[4];

    int num_verts;
    int num_tris;
    int num_weights;

    char shader[256];
};

/* MD5 model structure */
struct md5_model_t
{
    struct md5_joint_t *baseSkel;
    struct md5_mesh_t *meshes;

    int num_joints;
    int num_meshes;
};

/* Animation data */
struct md5_anim_t
{
    int num_frames;
    int num_joints;
    int frameRate;

    struct md5_joint_t **skelFrames;
    struct md5_bbox_t *bboxes;
};

/* Animation info */
struct anim_info_t
{
    int curr_frame;
    int next_frame;

    double last_time;
    double max_time;
};

/**
 * Quaternion prototypes
 */
void quat_computeW(quat4_t q);
void quat_normalize(quat4_t q);
void quat_multQuat(const quat4_t qa, const quat4_t qb, quat4_t out);
void quat_multVec(const quat4_t q, const vec3_t v, quat4_t out);
void quat_rotatePoint(const quat4_t q, const vec3_t in, vec3_t out);
float quat_dotProduct(const quat4_t qa, const quat4_t qb);
void quat_slerp(const quat4_t qa, const quat4_t qb, float t, quat4_t out);

/**
 * md5mesh prototypes
 */
int read_MD5_model(const char *filename, struct md5_model_t *mdl);
void free_model(struct md5_model_t *mdl);
void prepare_mesh(const struct md5_mesh_t *mesh,
                  const struct md5_joint_t *skeleton);
void alloc_vertex_arrays();
void free_vertex_arrays();
void draw_skeleton(const struct md5_joint_t *skeleton, int num_joints);
void draw_mesh(const struct md5_mesh_t *mesh);

/**
 * md5anim prototypes
 */
int check_anim_validity(const struct md5_model_t *mdl,
                        const struct md5_anim_t *anim);
int read_MD5_anim(const char *filename, struct md5_anim_t *anim);
void build_frame_skeleton(const struct joint_info_t *jointInfos,
                          const struct baseframe_joint_t *baseFrame,
                          const float *animFrameData,
                          struct md5_joint_t *skelFrame,
                          int num_joints);
void free_anim(struct md5_anim_t *anim);
void interpolate_skeletons(const struct md5_joint_t *skelA,
                           const struct md5_joint_t *skelB,
                           int num_joints, float interp,
                           struct md5_joint_t *out);
void animate(const struct md5_anim_t *anim,
             struct anim_info_t *animInfo, double dt);

#endif /* __MD5MODEL_H__ */

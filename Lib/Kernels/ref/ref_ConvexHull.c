/*
    File: ref_ConvexHull.c
    Реализация ConvexHull
    Date: 20 Августа 2021
*/

#include "../ref.h"

vx_coordinates2d_t* sign_points(vx_coordinates2d_t* arr, vx_coordinates2d_t* left,
    vx_coordinates2d_t* right, vx_size sign);
vx_coordinates2d_t dop_point(vx_coordinates2d_t*, vx_coordinates2d_t*,
    vx_coordinates2d_t*);
vx_float32 distance(vx_coordinates2d_t*, vx_coordinates2d_t*,
    vx_coordinates2d_t*);
void findHullUp(vx_coordinates2d_t*, vx_coordinates2d_t*,
    vx_coordinates2d_t*, uint32_t*, vx_coordinates2d_t*);
void findHullDown(vx_coordinates2d_t*, vx_coordinates2d_t*,
    vx_coordinates2d_t*, uint32_t*, vx_coordinates2d_t*);
int comp(vx_coordinates2d_t* i, vx_coordinates2d_t* j);
vx_float32 midx = 0;
vx_float32 midy = 0;

vx_status ref_ConvexHull(const vx_image src_image,
    vx_coordinates2d_t* dst, size_t* dst_size)
{
    const uint32_t src_width = src_image->width;
    const uint32_t src_height = src_image->height;

    if (src_width <= 0 || src_height <= 0)
    {
        return VX_ERROR_INVALID_PARAMETERS;
    }
    const vx_int8* src_data = src_image->data;
    vx_coordinates2d_t* points = (vx_coordinates2d_t*)malloc((src_width * src_height + 1) * sizeof(vx_coordinates2d_t));
    vx_size points_size = 0;
    for (vx_size ind = 0; ind < src_width * src_height; ++ind)
    {
        if (src_data[ind] != 0) {
            points[points_size].x = ind % src_width;
            points[points_size].y = ind / src_width;
            ++points_size;
        }
    }
    vx_coordinates2d_t max = { .x = 0, .y = 0 };
    vx_coordinates2d_t min = { .x = src_height, .y = src_width };
    for (vx_size ind = 0; ind < points_size; ++ind)
    {
        if (points[ind].x < min.x) {
            min.x = points[ind].x;
            min.y = points[ind].y;
        }
        if (points[ind].x > max.x) {
            max.x = points[ind].x;
            max.y = points[ind].y;
        }
    }
    dst[0] = min;
    dst[1] = max;
    vx_size out_points_size = 2;
    points[points_size].x = 0;
    points[points_size].y = 0;
    findHullUp(sign_points(points, &min, &max, 1), &min, &max, &out_points_size, dst);
    findHullDown(sign_points(points, &min, &max, -1), &min, &max, &out_points_size, dst);
    vx_size ind = 0;
    *dst_size = out_points_size;
    midx = (min.x + max.x) / 2;
    midy = (min.y + max.y) / 2;
    qsort(dst, *dst_size, sizeof(vx_coordinates2d_t), comp);
    return VX_SUCCESS;
}

int comp(vx_coordinates2d_t* i, vx_coordinates2d_t* j)
{
    vx_float32 px = (vx_float32)i->x - midx;
    vx_float32 py = (vx_float32)i->y - midy;
    vx_float32 qx = (vx_float32)(j->x) - (vx_float32)(i->x);
    vx_float32 qy = (vx_float32)(j->y) - (vx_float32)(i->y);
    if (px * qy == qx * py)
        return ((i->x) * (i->x) + (i->y) * (i->y) > (j->x) * (j->x) + (j->y) * (j->y));
    if (px * qy > qx * py) return 1;
    return -1;
}

vx_coordinates2d_t* sign_points(vx_coordinates2d_t* arr, vx_coordinates2d_t* left,
    vx_coordinates2d_t* right, vx_size sign)
{
    vx_size size = 0;
    for (vx_size ind = 0; (arr)[ind].x != 0 || (arr)[ind].y != 0; ++ind, ++size) {}
    vx_size index = 0;
    vx_coordinates2d_t* down_points = (vx_coordinates2d_t*)malloc(size * sizeof(vx_coordinates2d_t));
    if (sign == 1) {
        for (vx_size ind = 0; (arr)[ind].x != 0 || (arr)[ind].y != 0; ++ind) {
            if ((int)((right->x - left->x) * ((arr)[ind].y - left->y) - ((arr)[ind].x - left->x) * (right->y - left->y)) > 0) {
                down_points[index] = (arr)[ind];
                ++index;
            }
        }
    }
    if (sign == -1) {
        for (vx_size ind = 0; (arr)[ind].x != 0 || (arr)[ind].y != 0; ++ind) {
            if ((int)((right->x - left->x) * ((arr)[ind].y - left->y) - ((arr)[ind].x - left->x) * (right->y - left->y)) < 0) {
                down_points[index] = (arr)[ind];
                ++index;
            }
        }
    }
    down_points[index].x = 0;
    down_points[index].y = 0;
    if (index == 0) down_points = 0;
    return down_points;
}

vx_coordinates2d_t dop_point(vx_coordinates2d_t* arr, vx_coordinates2d_t* left,
    vx_coordinates2d_t* right) {
    vx_float32 max_dist = 0;
    vx_coordinates2d_t point = { .x = 0, .y = 0 };
    for (vx_size i = 0; (arr)[i].x != 0 || (arr)[i].y != 0; ++i) {
        vx_float32 tmp_dist = distance(&(arr[i]), left, right);
        if (tmp_dist > max_dist) { max_dist = tmp_dist; point = (arr)[i]; }
    }
    return point;
}

vx_float32 distance(vx_coordinates2d_t* p, vx_coordinates2d_t* p1,
    vx_coordinates2d_t* p2) {
    vx_float32 res = abs(((vx_float32)(p2->y) - (vx_float32)(p1->y)) * ((vx_float32)((p)->x)) - ((vx_float32)(p2->x) - (vx_float32)(p1->x))
        * ((vx_float32)((p)->y))
        + (vx_float32)(p2->x) * (vx_float32)(p1->y) - (vx_float32)(p2->y) * (vx_float32)(p1->x))
        / sqrt(pow((vx_float32)(p2->y) - (vx_float32)(p1->y), 2) + pow((vx_float32)(p2->x) - (vx_float32)(p1->x), 2));
    return res;
}

void findHullUp(vx_coordinates2d_t* points, vx_coordinates2d_t* left,
    vx_coordinates2d_t* right, vx_size* out_size, vx_coordinates2d_t* out) {
    if (points == 0) return;
    vx_coordinates2d_t middle = dop_point(points, left, right);
    out[*out_size] = middle;
    ++(*out_size);
    findHullUp(sign_points(points, left, &middle, 1), left, &middle, out_size, out);
    findHullUp(sign_points(points, &middle, right, 1), &middle, right, out_size, out);

}
void findHullDown(vx_coordinates2d_t* points, vx_coordinates2d_t* left,
    vx_coordinates2d_t* right, vx_size* out_size, vx_coordinates2d_t* out) {
    if (points == 0) return;
    vx_coordinates2d_t middle = dop_point(points, left, right);
    out[*out_size] = middle;
    ++(*out_size);
    findHullDown(sign_points(points, left, &middle, -1), left, &middle, out_size, out);
    findHullDown(sign_points(points, &middle, right, -1), &middle, right, out_size, out);

}
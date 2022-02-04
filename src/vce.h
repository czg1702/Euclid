#ifndef EUCLID__VCE_H
#define EUCLID__VCE_H 1

#include "command.h"
#include "utils.h"
#include "rb-tree.h"

#define SPACE_DEF_PARTITION_COUNT 128

typedef struct _coordinate_system
{
    // TODO
    __uint64_t id;
    ArrayList *axes;
} CoordinateSystem;

CoordinateSystem *cs_create(__uint64_t *id_addr);

typedef struct _coordinate_axis
{
    // TODO
    RedBlackTree *rbtree;
} Axis;

Axis *ax_create();

void ax_reordering(Axis *axis);
int ax_size(Axis *axis);

Axis *cs_get_axis(CoordinateSystem *cs, int axis_position);

void cs_add_axis(CoordinateSystem *cs, Axis *axis);

typedef struct _axis_scale
{
    __uint64_t *fragments;
    int fragments_len;
} Scale;

Scale *scal_create();

void *scal__destory(void *scale);

int scal_cmp(void *_one, void *_other);

// void scal_set_len(Scale *scale, int fgs_len);

void scal_put_fragments(Scale *scale, int fgs_len, void *fragments);

Scale *scal__alloc(int fgs_len, void *fragments);

void ax_set_scale(Axis *axis, Scale *scale);

int vce_append(EuclidCommand *ec);

void vce_init();

typedef struct _measure_space_
{
    // TODO
    __uint64_t id;
    size_t segment_count;
    size_t segment_scope;
    RedBlackTree **tree_ls_h;
    void **data_ls_h;
    int cell_vals_count;
} MeasureSpace;

void space_unload(__uint64_t id);

MeasureSpace *space_create(size_t segment_count, size_t segment_scope, int cell_vals_count);

__uint64_t ax_scale_position(Axis *axis, int fgs_len, void *fragments);

__uint64_t cs_axis_span(CoordinateSystem *cs, int axis_order);

void space_add_measure(MeasureSpace *space, __uint64_t measure_position, void *cell);

void space_plan(MeasureSpace *space);

#endif
/*
 * SAQueue.h
 * Copyright (C) 2011 by Chenguang Wang(wecing)
 *
 * This file is part of SimpleAV.
 *
 * SimpleAV is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * SimpleAV is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with SimpleAV. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __SIMPLEAV_CORE_UTIL_SAQUEUE_H_DEFINED__
#define __SIMPLEAV_CORE_UTIL_SAQUEUE_H_DEFINED__

typedef struct _SAQNode {
     void *data;
     struct _SAQNode *next;
} _SAQNode;

typedef struct SAQContext {
     _SAQNode *head, *tail;
     int nb;
} SAQContext;

SAQContext *SAQ_init(void);

int SAQ_push(SAQContext *, void *);

void *SAQ_pop(SAQContext *);

#endif

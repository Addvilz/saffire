/*
 Copyright (c) 2012-2013, The Saffire Group
 All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:
     * Redistributions of source code must retain the above copyright
       notice, this list of conditions and the following disclaimer.
     * Redistributions in binary form must reproduce the above copyright
       notice, this list of conditions and the following disclaimer in the
       documentation and/or other materials provided with the distribution.
     * Neither the name of the Saffire Group the
       names of its contributors may be used to endorse or promote products
       derived from this software without specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY
 DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "objects/object.h"
#include "objects/objects.h"
#include "general/smm.h"
#include "general/md5.h"
#include "debug.h"


/* ======================================================================
 *   Supporting functions
 * ======================================================================
 */


/* ======================================================================
 *   Object methods
 * ======================================================================
 */


/**
 * Saffire method: constructor
 */
SAFFIRE_METHOD(callable, ctor) {
    RETURN_SELF;
}

/**
 * Saffire method: destructor
 */
SAFFIRE_METHOD(callable, dtor) {
    RETURN_NULL;
}


/**
 *
 */
SAFFIRE_METHOD(callable, call) {
    // @TODO: Will do a call to the callable object
    RETURN_NULL;
}

/**
 *
 */
SAFFIRE_METHOD(callable, internal) {
    if (CALLABLE_IS_CODE_INTERNAL(self)) {
        RETURN_TRUE;
    }

    RETURN_FALSE;
}

/**
 * Rebinds the callable to another object
 */
SAFFIRE_METHOD(callable, bind) {
    // @TODO: use object_parse_parameters

    t_dll_element *e = DLL_HEAD(SAFFIRE_METHOD_ARGS);
    t_object *newbound_obj = (t_object *)e->data;
    self->binding = newbound_obj;

    RETURN_SELF;
}


/**
 *
 */
SAFFIRE_METHOD(callable, conv_boolean) {
    RETURN_TRUE;
}


/**
 *
 */
SAFFIRE_METHOD(callable, conv_null) {
    RETURN_NULL;
}


/* ======================================================================
 *   Standard operators
 * ======================================================================
 */


/* ======================================================================
 *   Standard comparisons
 * ======================================================================
 */


/* ======================================================================
 *   Global object management functions and data
 * ======================================================================
 */

/**
 * Initializes methods and properties, these are used
 */
void object_callable_init(void) {
    Object_Callable_struct.attributes = ht_create();
    object_add_internal_method((t_object *)&Object_Callable_struct, "__ctor",         CALLABLE_FLAG_NONE, ATTRIB_VISIBILITY_PUBLIC, object_callable_method_ctor);
    object_add_internal_method((t_object *)&Object_Callable_struct, "__dtor",         CALLABLE_FLAG_NONE, ATTRIB_VISIBILITY_PUBLIC, object_callable_method_dtor);

    object_add_internal_method((t_object *)&Object_Callable_struct, "__boolean",      CALLABLE_FLAG_NONE, ATTRIB_VISIBILITY_PUBLIC, object_callable_method_conv_boolean);
    object_add_internal_method((t_object *)&Object_Callable_struct, "__null",         CALLABLE_FLAG_NONE, ATTRIB_VISIBILITY_PUBLIC, object_callable_method_conv_null);

    object_add_internal_method((t_object *)&Object_Callable_struct, "__internal?",    CALLABLE_FLAG_NONE, ATTRIB_VISIBILITY_PUBLIC, object_callable_method_internal);
    object_add_internal_method((t_object *)&Object_Callable_struct, "__bind",         0, ATTRIB_VISIBILITY_PUBLIC, object_callable_method_bind);
}

/**
 * Frees memory for a callable object
 */
void object_callable_fini(void) {
    // Free attributes
    object_remove_all_internal_attributes((t_object *)&Object_Callable_struct);
    ht_destroy(Object_Callable_struct.attributes);
}


static t_object *obj_new(t_object *self) {
    // Create new object and copy all info
    t_callable_object *obj = smm_malloc(sizeof(t_callable_object));
    memcpy(obj, Object_Callable, sizeof(t_callable_object));

    // Dynamically allocated
    obj->flags |= OBJECT_FLAG_ALLOCATED;

    // These are instances
    obj->flags &= ~OBJECT_TYPE_MASK;
    obj->flags |= OBJECT_TYPE_INSTANCE;

    return (t_object *)obj;
}

static void obj_populate(t_object *obj, t_dll *arg_list) {
    t_callable_object *callable_obj = (t_callable_object *)obj;

    t_dll_element *e = DLL_HEAD(arg_list);
    callable_obj->callable_flags = (int)e->data;
    e = DLL_NEXT(e);


    if (CALLABLE_IS_CODE_INTERNAL(callable_obj)) {
        callable_obj->code.native_func = (void *)e->data;
        e = DLL_NEXT(e);
    } else {
        callable_obj->code.bytecode = (t_bytecode *)e->data;
        e = DLL_NEXT(e);
    }

    callable_obj->binding = (t_object *)e->data;
//    object_inc_ref(callable_obj->binding);
    e = DLL_NEXT(e);

    callable_obj->arguments = (t_hash_object *)e->data;
    e = DLL_NEXT(e);

    // @TODO: Something else...
    e = DLL_NEXT(e);


    // @TODO: remove me, a callable has no name directly attached to it
    if (e && e->data) {
        callable_obj->call_name = smm_strdup(e->data);
    } else {
        callable_obj->call_name = smm_strdup("<no name>");
    }
    if (e) e = DLL_NEXT(e);
}

static void obj_destroy(t_object *obj) {
    smm_free(obj);
}

static void obj_free(t_object *obj) {
    t_callable_object *callable_obj = (t_callable_object *)obj;
    object_release(callable_obj->binding);
}


#ifdef __DEBUG
char global_buf[1024];
static char *obj_debug(t_object *obj) {
    t_callable_object *self = (t_callable_object *)obj;
    sprintf(global_buf, "callable (%-15s) F:[%s%s%s%s%s%s]",
        self->call_name,
        (self->callable_flags & CALLABLE_FLAG_STATIC) == CALLABLE_FLAG_NONE ? "S" : "-",
        (self->callable_flags & CALLABLE_FLAG_ABSTRACT) == CALLABLE_FLAG_ABSTRACT ? "A" : "-",
        (self->callable_flags & CALLABLE_FLAG_FINAL) == CALLABLE_FLAG_FINAL ? "F" : "-",
        (self->callable_flags & CALLABLE_FLAG_CONSTRUCTOR) == CALLABLE_FLAG_CONSTRUCTOR ? "C" : (self->callable_flags & CALLABLE_FLAG_DESTRUCTOR) == CALLABLE_FLAG_DESTRUCTOR ? "D" : "-",
        (self->callable_flags & CALLABLE_TYPE_METHOD) == CALLABLE_TYPE_METHOD ? "M" : "-",
        (self->callable_flags & CALLABLE_CODE_INTERNAL) == CALLABLE_CODE_INTERNAL ? "I" : (self->callable_flags & CALLABLE_CODE_EXTERNAL) == CALLABLE_CODE_EXTERNAL ? "E" : "-"
    );



    return global_buf;
}

#endif


// Callable object management functions
t_object_funcs callable_funcs = {
        obj_new,              // Allocate a new callable object
        obj_populate,         // Populates a callable object
        obj_free,             // Free a callable object
        obj_destroy,          // Destroy a callable object
        NULL,                 // Clone
        NULL,                 // Cache
#ifdef __DEBUG
        obj_debug
#endif
};

// Initial object
t_callable_object Object_Callable_struct = {
    OBJECT_HEAD_INIT("callable", objectTypeCallable, OBJECT_TYPE_CLASS, &callable_funcs),
    0,
    { NULL },
    NULL,
    NULL
};

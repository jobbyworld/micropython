/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2016 Colin Hogben
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
#include "py/runtime.h"
#include "py/runtime0.h"
#include "mreg.h"
#include <string.h>

STATIC mp_obj_t mreg_array_new(void *ptr, const mreg_desc_t *desc);
STATIC void mreg_array_print_helper(const mp_print_t *print, char *ptr, const mreg_desc_t *desc, mp_print_kind_t kind);

/*-----------------------------------------------------------------------
 *      MregStruct
 *-----------------------------------------------------------------------*/
STATIC mp_obj_t mreg_struct_new(void *ptr, const mreg_field_t *fields) {
    mreg_struct_obj_t *o = m_new_obj(mreg_struct_obj_t);
    o->base.type = &mreg_struct_type;
    o->ptr = ptr;
    o->fields = fields;
    return o;
}

// Get [NULL], set [SEN, v] or delete [SEN, NULL] an attribute
STATIC void mreg_struct_attr(mp_obj_t self_in, qstr qattr, mp_obj_t *dest) {
    mreg_struct_obj_t *self = (mreg_struct_obj_t *)self_in;
    if (qattr == MP_QSTR_address) {
        if (dest[0] != MP_OBJ_NULL) return; /* Cannot set or delete address */
        dest[0] = mp_obj_new_int((mp_uint_t)self->ptr);
        return;
    }
    const char *attr = qstr_str(qattr);
    const mreg_field_t *field;
    for (field=self->fields; field->name; field++) {
        if (! strcmp(field->name, attr)) {
            /* Found named member */
            char *fptr = self->ptr + field->offset;
            const mreg_desc_t *desc = &field->desc;
            if (dest[0] == MP_OBJ_NULL) { /* Get */
                if (desc->len != 0) {
                    // Array
                    dest[0] = mreg_array_new(fptr, desc);
                } else {
                    switch (desc->type) {
                    case MREG_TYPE_STRUCT:
                        // Sub-structure
                        dest[0] = mreg_struct_new(fptr, desc->itemtype);
                        break;
                    case MREG_TYPE_U8:;
                        uint8_t value8 = *(uint8_t *)fptr;
                        dest[0] = mp_obj_new_int(value8);
                        break;
                    case MREG_TYPE_U16:;
                        uint16_t value16 = *(uint16_t *)fptr;
                        dest[0] = mp_obj_new_int(value16);
                        break;
                    case MREG_TYPE_U32:;
                        uint32_t value32 = *(uint32_t *)fptr;
                        dest[0] = mp_obj_new_int(value32);
                        break;
                    }
                }
                return;
            } else {                    /* Set or delete */
                if (dest[1] == MP_OBJ_NULL) return; /* Cannot delete */
                mp_int_t value = mp_obj_get_int(dest[1]);
                if (desc->len != 0) return; /* Cannot set array */
                switch (desc->type) {
                case MREG_TYPE_STRUCT:
                    // Sub-structure
                    return;             /* Cannot set srtuct */
                case MREG_TYPE_U8:
                    *(uint8_t *)fptr = value;
                    break;
                case MREG_TYPE_U16:
                    *(uint16_t *)fptr = value;
                    break;
                case MREG_TYPE_U32:
                    *(uint32_t *)fptr = value;
                    break;
                }
                dest[0] = MP_OBJ_NULL;  /* Successfully set */
            }
            return;
        }
    }
    // Not found, return (caller will raise exception)
}

// Similar to dict_print
STATIC void mreg_struct_print_helper(const mp_print_t *print, char *ptr, const mreg_field_t *fields, mp_print_kind_t kind) {
    if (!(MICROPY_PY_UJSON && kind == PRINT_JSON)) {
        kind = PRINT_REPR;
    }
    mp_print_str(print, "{");
    bool first = true;
    for (const mreg_field_t *field = fields; field->name; field++) {
        if (! first) {
            mp_print_str(print, ", ");
        }
        first = false;
        char *fptr = ptr + field->offset;
        mp_printf(print, "'%s': ", field->name);
        if (field->desc.len != 0) {
            mreg_array_print_helper(print, fptr, &field->desc, kind);
        } else {
            switch (field->desc.type) {
            case MREG_TYPE_STRUCT:
                mreg_struct_print_helper(print, fptr, field->desc.itemtype, kind);
                break;
            case MREG_TYPE_U8:;
                unsigned int value8 = *(uint8_t *)fptr;
                mp_printf(print, "%u", value8);
                break;
            case MREG_TYPE_U16:;
                unsigned int value16 = *(uint16_t *)fptr;
                mp_printf(print, "%u", value16);
                break;
            case MREG_TYPE_U32:;
                unsigned int value32 = *(uint32_t *)fptr;
                mp_printf(print, "%u", value32);
                break;
            }
        }
    }
    mp_print_str(print, "}");
}

STATIC void mreg_struct_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind) {
    mreg_struct_obj_t *self = MP_OBJ_TO_PTR(self_in);
    mreg_struct_print_helper(print, self->ptr, self->fields, kind);
}

const mp_obj_type_t mreg_struct_type = {
    { &mp_type_type },
    .name = MP_QSTR_MregStruct,
    .print = mreg_struct_print,
    .attr = mreg_struct_attr,
};

/*-----------------------------------------------------------------------
 *      MregArray
 *-----------------------------------------------------------------------*/
const mp_obj_type_t mreg_array_type;

STATIC mp_obj_t mreg_array_new(void *ptr, const mreg_desc_t *desc) {
    mreg_array_obj_t *o = m_new_obj(mreg_array_obj_t);
    o->base.type = &mreg_array_type;
    o->ptr = ptr;
    o->desc = desc;
    return o;
}

// Get, set or delete an attribute
STATIC void mreg_array_attr(mp_obj_t self_in, qstr qattr, mp_obj_t *dest) {
    mreg_array_obj_t *self = (mreg_array_obj_t *)self_in;
    if (dest[0] != MP_OBJ_NULL) {
        return;                         /* FAIL for store or delete */
    }
    if (qattr == MP_QSTR_address) {
        dest[0] = mp_obj_new_int((mp_uint_t)self->ptr);
        return;
    }
}

// Get, set or delete an array item
STATIC mp_obj_t mreg_array_subscr(mp_obj_t self_in, mp_obj_t index_in, mp_obj_t value) {
    mreg_array_obj_t *self = (mreg_array_obj_t *)self_in;
    if (value == MP_OBJ_NULL) {  /* SENTINEL means get, NULL delete */
        return MP_OBJ_NULL;             /* op not supported */
    }
    mp_int_t index = mp_obj_get_int(index_in);
    if (index < 0) index += self->desc->len;
    if (! (0 <= index && index < (int)self->desc->len)) {
        nlr_raise(&mp_type_IndexError);
    }
    if (value == MP_OBJ_SENTINEL) {     /* Get */
        switch (self->desc->type) {
        case MREG_TYPE_STRUCT:;
            char *fptr = self->ptr + (index * self->desc->stride);
            return mreg_struct_new(fptr, self->desc->itemtype);
        case MREG_TYPE_U8:;
            uint8_t *array8 = (uint8_t *)self->ptr;
            uint8_t value8 = array8[index];
            return mp_obj_new_int(value8);
        case MREG_TYPE_U16:;
            uint16_t *array16 = (uint16_t *)self->ptr;
            uint16_t value16 = array16[index];
            return mp_obj_new_int(value16);
        case MREG_TYPE_U32:;
            uint32_t *array32 = (uint32_t *)self->ptr;
            uint32_t value32 = array32[index];
            return mp_obj_new_int(value32);
        }
        return mp_const_none;           /* FIXME assert? */
    } else {                            /* Set */
        mp_int_t ivalue = mp_obj_get_int(value);
        switch (self->desc->type) {
        case MREG_TYPE_STRUCT:
            return MP_OBJ_NULL;         /* Cannot set whole struct */
        case MREG_TYPE_U8:;
            uint8_t *array8 = (uint8_t *)self->ptr;
            array8[index] = ivalue;
            break;
        case MREG_TYPE_U16:;
            uint16_t *array16 = (uint16_t *)self->ptr;
            array16[index] = ivalue;
            break;
        case MREG_TYPE_U32:;
            uint32_t *array32 = (uint32_t *)self->ptr;
            array32[index] = ivalue;
            break;
        }
        return mp_const_none;
    }
}

// Similar to tuple_print
STATIC void mreg_array_print_helper(const mp_print_t *print, char *ptr, const mreg_desc_t *desc, mp_print_kind_t kind) {
    if (MICROPY_PY_UJSON && kind == PRINT_JSON) {
        mp_print_str(print, "[");
    } else {
        mp_print_str(print, "(");
        kind = PRINT_REPR;
    }
    mp_uint_t i;
    for (i = 0; i < desc->len; i++) {
        char *fptr = ptr + i * desc->stride;
        if (i > 0) {
            mp_print_str(print, ", ");
        }
        switch (desc->type) {
        case MREG_TYPE_STRUCT:
            mreg_struct_print_helper(print, fptr, desc->itemtype, kind);
            break;
        case MREG_TYPE_U8:;
            unsigned int value8 = *(uint8_t *)fptr;
            mp_printf(print, "%u", value8);
            break;
        case MREG_TYPE_U16:;
            unsigned int value16 = *(uint16_t *)fptr;
            mp_printf(print, "%u", value16);
            break;
        case MREG_TYPE_U32:;
            unsigned int value32 = *(uint32_t *)fptr;
            mp_printf(print, "%u", value32);
            break;
        }
    }
    if (MICROPY_PY_UJSON && kind == PRINT_JSON) {
        mp_print_str(print, "]");
    } else {
        if (i == 1) {
            mp_print_str(print, ",");
        }
        mp_print_str(print, ")");
    }
}

STATIC void mreg_array_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind) {
    mreg_array_obj_t *self = MP_OBJ_TO_PTR(self_in);
    mreg_array_print_helper(print, self->ptr, self->desc, kind);
}

STATIC mp_obj_t mreg_array_unary_op(mp_unary_op_t op, mp_obj_t self_in) {
    mreg_array_obj_t *self = MP_OBJ_TO_PTR(self_in);
    switch (op) {
    case MP_UNARY_OP_LEN:
        return MP_OBJ_NEW_SMALL_INT(self->desc->len);
    default:
        return MP_OBJ_NULL;             // op not supported
    }
}

const mp_obj_type_t mreg_array_type = {
    { &mp_type_type },
    .name = MP_QSTR_MregArray,
    .print = mreg_array_print,
    .attr = mreg_array_attr,
    .subscr = mreg_array_subscr,
    .unary_op = mreg_array_unary_op,
};

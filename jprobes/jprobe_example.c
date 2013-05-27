/*
 * Here's a sample kernel module showing the use of jprobes to dump
 * the arguments of do_fork().
 *
 * For more information on theory of operation of jprobes, see
 * Documentation/kprobes.txt
 *
 * Build and insert the kernel module as done in the kprobe example.
 * You will see the trace data in /var/log/messages and on the
 * console whenever do_fork() is invoked to create a new process.
 * (Some messages may be suppressed if syslogd is configured to
 * eliminate duplicate messages.)
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/kprobes.h>
#include <acpi/acpi.h>
#include "acpica/accommon.h"
#include "acpica/amlcode.h"

#define MAX_LEVEL (16)
static int cur_level;

//static int my_snprintf(char * pc, int len, const char * fmt, ...)
//{
//    va_list args;
//    int r;
//
//    va_start(args, fmt);
//    r = vsnprintf(fmt, args);
//    va_end(args);
//
//    return r;
//
//}


static int
acpi_ex_dump_operand(char * buf, int maxlen, union acpi_operand_object *obj_desc, u32 depth)
{
	u32 length;
    int str_len = 0;

	ACPI_FUNCTION_NAME(ex_dump_operand)

    if (maxlen <= 0) {
        return maxlen;
    }

	if (!obj_desc) {

		/* This could be a null element of a package */

		ACPI_DEBUG_PRINT((ACPI_DB_EXEC, "Null Object Descriptor\n"));
		return maxlen;
	}

	if (ACPI_GET_DESCRIPTOR_TYPE(obj_desc) == ACPI_DESC_TYPE_NAMED) {
		ACPI_DEBUG_PRINT((ACPI_DB_EXEC, "%p Namespace Node: ",
				  obj_desc));
		ACPI_DUMP_ENTRY(obj_desc, ACPI_LV_EXEC);
		return maxlen;
	}

	if (ACPI_GET_DESCRIPTOR_TYPE(obj_desc) != ACPI_DESC_TYPE_OPERAND) {
		ACPI_DEBUG_PRINT((ACPI_DB_EXEC,
				  "%p is not a node or operand object: [%s]\n",
				  obj_desc,
				  acpi_ut_get_descriptor_name(obj_desc)));
		ACPI_DUMP_BUFFER(obj_desc, sizeof(union acpi_operand_object));
		return maxlen;
	}

	/* obj_desc is a valid object */

	if (depth > 0) {
		ACPI_DEBUG_PRINT((ACPI_DB_EXEC, "%*s[%u] %p ",
				  depth, " ", depth, obj_desc));
	} else {
		ACPI_DEBUG_PRINT((ACPI_DB_EXEC, "%p ", obj_desc));
	}

	/* Decode object type */

	switch (obj_desc->common.type) {
#if 0
	case ACPI_TYPE_LOCAL_REFERENCE:

		//str_len += snprintf(buf + str_len, maxlen - str_len, "Reference: [%s] ",
        //                    acpi_ut_get_reference_name(obj_desc));

		switch (obj_desc->reference.class) {
		case ACPI_REFCLASS_DEBUG:

			str_len += snprintf(buf + str_len, maxlen - str_len, "\n");
			break;

		case ACPI_REFCLASS_INDEX:

			str_len += snprintf(buf + str_len, maxlen - str_len, "%p\n", obj_desc->reference.object);
			break;

		case ACPI_REFCLASS_TABLE:

			str_len += snprintf(buf + str_len, maxlen - str_len, "Table Index %X\n",
                                obj_desc->reference.value);
			break;

		case ACPI_REFCLASS_REFOF:

			//str_len += snprintf(buf + str_len, maxlen - str_len, "%p [%s]\n", obj_desc->reference.object,
            //                     acpi_ut_get_type_name(((union
            //                                 acpi_operand_object
            //                                 *)
            //                                obj_desc->
            //                                reference.
            //                                object)->common.
            //                               type));
			break;

		case ACPI_REFCLASS_NAME:

			str_len += snprintf(buf + str_len, maxlen - str_len, "- [%4.4s]\n",
                               obj_desc->reference.node->name.ascii);
			break;

		case ACPI_REFCLASS_ARG:
		case ACPI_REFCLASS_LOCAL:

			str_len += snprintf(buf + str_len, maxlen - str_len, "%X\n", obj_desc->reference.value);
			break;

		default:	/* Unknown reference class */

			str_len += snprintf(buf + str_len, maxlen - str_len, "%2.2X\n", obj_desc->reference.class);
			break;
		}
		break;

	case ACPI_TYPE_BUFFER:

		str_len += snprintf(buf + str_len, maxlen - str_len, "Buffer length %.2X @ %p\n",
			       obj_desc->buffer.length,
			       obj_desc->buffer.pointer);

		/* Debug only -- dump the buffer contents */

		if (obj_desc->buffer.pointer) {
			length = obj_desc->buffer.length;
			if (length > 128) {
				length = 128;
			}

			str_len += snprintf(buf + str_len, maxlen - str_len,
			    "Buffer Contents: (displaying length 0x%.2X)\n",
			     length);
			ACPI_DUMP_BUFFER(obj_desc->buffer.pointer, length);
		}
		break;
#endif

	case ACPI_TYPE_INTEGER:

		str_len += snprintf(buf + str_len, maxlen - str_len, "Integer %8.8X%8.8X\n",
			       ACPI_FORMAT_UINT64(obj_desc->integer.value));
		break;

#if 0
	case ACPI_TYPE_PACKAGE:

		str_len += snprintf(buf + str_len, maxlen - str_len, "Package [Len %X] ElementArray %p\n",
			       obj_desc->package.count,
			       obj_desc->package.elements);

		///*
		// * If elements exist, package element pointer is valid,
		// * and debug_level exceeds 1, dump package's elements.
		// */
		//if (obj_desc->package.count &&
		//    obj_desc->package.elements && acpi_dbg_level > 1) {
		//	for (index = 0; index < obj_desc->package.count; index++) {
		//		acpi_ex_dump_operand(obj_desc->package.
		//				     elements[index],
		//				     depth + 1);
		//	}
		//}
		break;

	case ACPI_TYPE_REGION:

		//str_len += snprintf(buf + str_len, maxlen - str_len, "Region %s (%X)",
		//	       acpi_ut_get_region_name(obj_desc->region.
		//				       space_id),
		//	       obj_desc->region.space_id);

		/*
		 * If the address and length have not been evaluated,
		 * don't print them.
		 */
		if (!(obj_desc->region.flags & AOPOBJ_DATA_VALID)) {
			str_len += snprintf(buf + str_len, maxlen - str_len, "\n");
		} else {
			str_len += snprintf(buf + str_len, maxlen - str_len, " base %8.8X%8.8X Length %X\n",
				       ACPI_FORMAT_NATIVE_UINT(obj_desc->region.
							       address),
				       obj_desc->region.length);
		}
		break;

	case ACPI_TYPE_STRING:

		str_len += snprintf(buf + str_len, maxlen - str_len, "String length %X @ %p ",
			       obj_desc->string.length,
			       obj_desc->string.pointer);

		//acpi_ut_print_string(obj_desc->string.pointer, ACPI_UINT8_MAX);
		str_len += snprintf(buf + str_len, maxlen - str_len, "\n");
		break;

	case ACPI_TYPE_LOCAL_BANK_FIELD:

		str_len += snprintf(buf + str_len, maxlen - str_len, "BankField\n");
		break;

	case ACPI_TYPE_LOCAL_REGION_FIELD:

		 str_len += snprintf(buf + str_len, maxlen - str_len, 
		    "RegionField: Bits=%X AccWidth=%X Lock=%X Update=%X at "
		     "byte=%X bit=%X of below:\n", obj_desc->field.bit_length,
		     obj_desc->field.access_byte_width,
		     obj_desc->field.field_flags & AML_FIELD_LOCK_RULE_MASK,
		     obj_desc->field.field_flags & AML_FIELD_UPDATE_RULE_MASK,
		     obj_desc->field.base_byte_offset,
		     obj_desc->field.start_field_bit_offset);

		//acpi_ex_dump_operand(obj_desc->field.region_obj, depth + 1);
		break;

	case ACPI_TYPE_LOCAL_INDEX_FIELD:

		str_len += snprintf(buf + str_len, maxlen - str_len, "IndexField\n");
		break;

	case ACPI_TYPE_BUFFER_FIELD:

		str_len += snprintf(buf + str_len, maxlen - str_len, "BufferField: %X bits at byte %X bit %X of\n",
			       obj_desc->buffer_field.bit_length,
			       obj_desc->buffer_field.base_byte_offset,
			       obj_desc->buffer_field.start_field_bit_offset);

		if (!obj_desc->buffer_field.buffer_obj) {
			ACPI_DEBUG_PRINT((ACPI_DB_EXEC, "*NULL*\n"));
		} else if ((obj_desc->buffer_field.buffer_obj)->common.type !=
			   ACPI_TYPE_BUFFER) {
			str_len += snprintf(buf + str_len, maxlen - str_len, "*not a Buffer*\n");
		} else {
			//acpi_ex_dump_operand(obj_desc->buffer_field.buffer_obj,
			//		     depth + 1);
		}
		break;

	case ACPI_TYPE_EVENT:

		str_len += snprintf(buf + str_len, maxlen - str_len, "Event\n");
		break;

	case ACPI_TYPE_METHOD:

		str_len += snprintf(buf + str_len, maxlen - str_len, "Method(%X) @ %p:%X\n",
			       obj_desc->method.param_count,
			       obj_desc->method.aml_start,
			       obj_desc->method.aml_length);
		break;

	case ACPI_TYPE_MUTEX:

		str_len += snprintf(buf + str_len, maxlen - str_len, "Mutex\n");
		break;

	case ACPI_TYPE_DEVICE:

		str_len += snprintf(buf + str_len, maxlen - str_len, "Device\n");
		break;

	case ACPI_TYPE_POWER:

		str_len += snprintf(buf + str_len, maxlen - str_len, "Power\n");
		break;

	case ACPI_TYPE_PROCESSOR:

		str_len += snprintf(buf + str_len, maxlen - str_len, "Processor\n");
		break;

	case ACPI_TYPE_THERMAL:

		str_len += snprintf(buf + str_len, maxlen - str_len, "Thermal\n");
		break;
#endif

	default:
		/* Unknown Type */

		str_len += snprintf(buf + str_len, maxlen - str_len, "Unknown Type %X\n", obj_desc->common.type);
		break;
	}

	return str_len;
}


acpi_status my_acpi_ds_init_aml_walk(struct acpi_walk_state *walk_state,
                 union acpi_parse_object *op,
                 struct acpi_namespace_node *method_node,
                 u8 * aml_start,
                 u32 aml_length,
                 struct acpi_evaluate_info *info, u8 pass_number);
void my_acpi_ds_terminate_control_method(union acpi_operand_object *method_desc,
				 struct acpi_walk_state *walk_state);

static struct jprobe my_jprobes[] = {
    {
        .entry              = my_acpi_ds_init_aml_walk,
        .kp = {
            .symbol_name    = "acpi_ds_init_aml_walk",
        },
    },
    {
        .entry              = my_acpi_ds_terminate_control_method,
        .kp = {
            .symbol_name    = "acpi_ds_terminate_control_method",
        },
    }

};

static void cleanup(void)
{
    int i;

    for (i = 0; i < sizeof(my_jprobes)/sizeof(struct jprobe); ++i) {
        if (my_jprobes[i].kp.addr != 0) {
            unregister_jprobe(&my_jprobes[i]);
        }
    }
}

static char * shift_mark = "................................................................";
static char cur_method_name[MAX_LEVEL][5];

acpi_status
my_acpi_ds_init_aml_walk(struct acpi_walk_state *walk_state,
		      union acpi_parse_object *op,
		      struct acpi_namespace_node *method_node,
		      u8 * aml_start,
		      u32 aml_length,
		      struct acpi_evaluate_info *info, u8 pass_number)
{
#define MAX_LEN (128)
    char buf[MAX_LEN] = {0};
    int len;
    int i = 0;

    if (!method_node) {
        jprobe_return();
        return 0;
    }
    if (cur_level++ >= MAX_LEVEL) {
        jprobe_return();
        return 0;
    }

    memcpy(cur_method_name[cur_level - 1], method_node->name.ascii, 4);
    printk("ASLP:%.*s%4.4s >\n", (cur_level - 1) * 2, shift_mark, 
           cur_method_name[cur_level - 1]);
    if (info && info->parameters) {
        //printk("ASLP:%.*s  info: %p, parameters: %p\n", (cur_level - 1) * 2, shift_mark, 
        //       info, info->parameters);
        for (i= 0; i < ACPI_METHOD_NUM_ARGS && info->parameters[i]; ++i) {

            len = acpi_ex_dump_operand(buf, MAX_LEN - 1, info->parameters[i], 0);
            if (len > 0) {
                printk("ASLP:%.*s  Op%d: %s", (cur_level - 1) * 2, shift_mark, 
                       i, buf);
            }
        }
    }
    jprobe_return();
    return 0;
}

void
my_acpi_ds_terminate_control_method(union acpi_operand_object *method_desc,
				 struct acpi_walk_state *walk_state)
{
    printk("ASLP:%.*s%4.4s <\n", (cur_level - 1) * 2, shift_mark, 
           cur_method_name[cur_level - 1]);
    if (--cur_level < 0) {
        cur_level = 0;
    }
    jprobe_return();
    return;
}

static int __init jprobe_init(void)
{
    int ret;
    int i;

    for (i = 0; i < sizeof(my_jprobes)/sizeof(struct jprobe); ++i) {
        ret = register_jprobe(&my_jprobes[i]);
        if (ret < 0) {
            printk(KERN_INFO "register_jprobe failed, returned %d for probe %d\n", ret, i);
            my_jprobes[i].kp.addr = 0;
            cleanup();
            return -1;
        } else {
            printk(KERN_INFO "Planted jprobe at %p, handler addr %p\n",
                   my_jprobes[i].kp.addr, my_jprobes[i].entry);
        }
    }

    return 0;
}

static void __exit jprobe_exit(void)
{
    cleanup();
}

module_init(jprobe_init)
module_exit(jprobe_exit)
MODULE_LICENSE("GPL");

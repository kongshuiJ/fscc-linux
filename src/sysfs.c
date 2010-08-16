/*
	Copyright (C) 2010  Commtech, Inc.
	
	This file is part of fscc-linux.

	fscc-linux is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	fscc-linux is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with fscc-linux.  If not, see <http://www.gnu.org/licenses/>.
	
*/

#include <linux/version.h>
#include "sysfs.h"
#include "utils.h" /* str_to_register_offset */

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 25)

static ssize_t register_store(struct kobject *kobj, struct kobj_attribute *attr,
                              const char *buf, size_t count, 
                              unsigned bar_number)
{
	struct fscc_port *port = 0;
	int register_offset = 0;
	unsigned value = 0;
    char *end = 0;
	
	port = (struct fscc_port *)dev_get_drvdata((struct device *)kobj);

	register_offset = str_to_register_offset(attr->attr.name);
	value = (unsigned)simple_strtoul(buf, &end, 16);

	if (register_offset >= 0) {
		dev_dbg(port->device, "setting register 0x%02x to 0x%08x\n",
		        register_offset, value);
		       
		fscc_port_set_register(port, bar_number, register_offset, value);
		return count;
	}

	return 0;
}

static ssize_t register_show(struct kobject *kobj, struct kobj_attribute *attr,
                             char *buf, unsigned bar_number)

{
	struct fscc_port *port = 0;
	int register_offset = 0;
	
	port = (struct fscc_port *)dev_get_drvdata((struct device *)kobj);

	register_offset = str_to_register_offset(attr->attr.name);
	
	if (register_offset >= 0) {
		dev_dbg(port->device, "reading register 0x%02x\n", register_offset);
		       
		return sprintf(buf, "%08x\n", fscc_port_get_register(port, bar_number, (unsigned)register_offset));
	}

	return 0;
}

static ssize_t bar0_register_store(struct kobject *kobj, 
                                   struct kobj_attribute *attr, const char *buf, 
                                   size_t count)
{
	return register_store(kobj, attr, buf, count, 0);
}

static ssize_t bar0_register_show(struct kobject *kobj, 
                                  struct kobj_attribute *attr, char *buf)
{
	return register_show(kobj, attr, buf, 0);
}

static ssize_t bar2_register_store(struct kobject *kobj, 
                                   struct kobj_attribute *attr, const char *buf, 
                                   size_t count)
{
	return register_store(kobj, attr, buf, count, 2);
}

static ssize_t bar2_register_show(struct kobject *kobj, 
                                  struct kobj_attribute *attr, char *buf)
{
	return register_show(kobj, attr, buf, 2);
}

static struct kobj_attribute fifot_attribute = 
	__ATTR(fifot, SYSFS_READ_WRITE_MODE, bar0_register_show, bar0_register_store);
	
static struct kobj_attribute cmdr_attribute = 
	__ATTR(cmdr, SYSFS_READ_WRITE_MODE, bar0_register_show, bar0_register_store);
	
static struct kobj_attribute ccr0_attribute = 
	__ATTR(ccr0, SYSFS_READ_WRITE_MODE, bar0_register_show, bar0_register_store);

static struct kobj_attribute ccr1_attribute = 
	__ATTR(ccr1, SYSFS_READ_WRITE_MODE, bar0_register_show, bar0_register_store);

static struct kobj_attribute ccr2_attribute = 
	__ATTR(ccr2, SYSFS_READ_WRITE_MODE, bar0_register_show, bar0_register_store);

static struct kobj_attribute bgr_attribute = 
	__ATTR(bgr, SYSFS_READ_WRITE_MODE, bar0_register_show, bar0_register_store);

static struct kobj_attribute ssr_attribute = 
	__ATTR(ssr, SYSFS_READ_WRITE_MODE, bar0_register_show, bar0_register_store);

static struct kobj_attribute smr_attribute = 
	__ATTR(smr, SYSFS_READ_WRITE_MODE, bar0_register_show, bar0_register_store);

static struct kobj_attribute tsr_attribute = 
	__ATTR(tsr, SYSFS_READ_WRITE_MODE, bar0_register_show, bar0_register_store);

static struct kobj_attribute tmr_attribute = 
	__ATTR(tmr, SYSFS_READ_WRITE_MODE, bar0_register_show, bar0_register_store);

static struct kobj_attribute rar_attribute = 
	__ATTR(rar, SYSFS_READ_WRITE_MODE, bar0_register_show, bar0_register_store);

static struct kobj_attribute ramr_attribute = 
	__ATTR(ramr, SYSFS_READ_WRITE_MODE, bar0_register_show, bar0_register_store);

static struct kobj_attribute ppr_attribute = 
	__ATTR(ppr, SYSFS_READ_WRITE_MODE, bar0_register_show, bar0_register_store);

static struct kobj_attribute tcr_attribute = 
	__ATTR(tcr, SYSFS_READ_WRITE_MODE, bar0_register_show, bar0_register_store);

static struct kobj_attribute vstr_attribute = 
	__ATTR(vstr, SYSFS_READ_ONLY_MODE, bar0_register_show, 0);

static struct kobj_attribute imr_attribute = 
	__ATTR(imr, SYSFS_READ_WRITE_MODE, bar0_register_show, bar0_register_store);

static struct kobj_attribute fcr_attribute = 
	__ATTR(fcr, SYSFS_READ_WRITE_MODE, bar2_register_show, bar2_register_store);

static struct attribute *register_attrs[] = {
	&fifot_attribute.attr,
	&cmdr_attribute.attr,
	&ccr0_attribute.attr,
	&ccr1_attribute.attr,
	&ccr2_attribute.attr,
	&bgr_attribute.attr,
	&ssr_attribute.attr,
	&smr_attribute.attr,
	&tsr_attribute.attr,
	&tmr_attribute.attr,
	&rar_attribute.attr,
	&ramr_attribute.attr,
	&ppr_attribute.attr,
	&tcr_attribute.attr,
	&vstr_attribute.attr,
	&imr_attribute.attr,
	&fcr_attribute.attr,
	NULL,
};

struct attribute_group port_registers_attr_group = {
	.name = "registers",
	.attrs = register_attrs,
};








static ssize_t flush_tx(struct kobject *kobj, struct kobj_attribute *attr,
                              const char *buf, size_t count)
{
	struct fscc_port *port = 0;
	
	port = (struct fscc_port *)dev_get_drvdata((struct device *)kobj);
	
	fscc_port_flush_tx(port);

	return count;
}

static ssize_t flush_rx(struct kobject *kobj, struct kobj_attribute *attr,
                              const char *buf, size_t count)
{
	struct fscc_port *port = 0;
	
	port = (struct fscc_port *)dev_get_drvdata((struct device *)kobj);
	
	fscc_port_flush_rx(port);

	return count;
}

static struct kobj_attribute flush_tx_attribute = 
	__ATTR(flush_tx, SYSFS_WRITE_ONLY_MODE, 0, flush_tx);

static struct kobj_attribute flush_rx_attribute = 
	__ATTR(flush_rx, SYSFS_WRITE_ONLY_MODE, 0, flush_rx);

static struct attribute *command_attrs[] = {
	&flush_tx_attribute.attr,
	&flush_rx_attribute.attr,
	NULL,
};

struct attribute_group port_commands_attr_group = {
	.name = "commands",
	.attrs = command_attrs,
};









static ssize_t output_memory(struct kobject *kobj, struct kobj_attribute *attr,
                             char *buf)
{
	struct fscc_port *port = 0;
	
	port = (struct fscc_port *)dev_get_drvdata((struct device *)kobj);
	
	return sprintf(buf, "%i\n", fscc_port_get_output_memory_usage(port));
}

static ssize_t input_memory(struct kobject *kobj, struct kobj_attribute *attr,
                                  char *buf)
{
	struct fscc_port *port = 0;
	
	port = (struct fscc_port *)dev_get_drvdata((struct device *)kobj);
	
	return sprintf(buf, "%i\n", fscc_port_get_input_memory_usage(port));
}

static ssize_t output_frames(struct kobject *kobj, struct kobj_attribute *attr,
                            char *buf)
{
	struct fscc_port *port = 0;
	
	port = (struct fscc_port *)dev_get_drvdata((struct device *)kobj);
	
	return sprintf(buf, "%i\n", fscc_port_get_output_frames_qty(port));
}

static ssize_t input_frames(struct kobject *kobj, struct kobj_attribute *attr,
                            char *buf)
{
	struct fscc_port *port = 0;
	
	port = (struct fscc_port *)dev_get_drvdata((struct device *)kobj);
	
	return sprintf(buf, "%i\n", fscc_port_get_input_frames_qty(port));
}

static struct kobj_attribute output_memory_attribute = 
	__ATTR(output_memory, SYSFS_READ_ONLY_MODE, output_memory, 0);

static struct kobj_attribute input_memory_attribute = 
	__ATTR(input_memory, SYSFS_READ_ONLY_MODE, input_memory, 0);

static struct kobj_attribute output_frames_attribute = 
	__ATTR(output_frames, SYSFS_READ_ONLY_MODE, output_frames, 0);

static struct kobj_attribute input_frames_attribute = 
	__ATTR(input_frames, SYSFS_READ_ONLY_MODE, input_frames, 0);

static struct attribute *info_attrs[] = {
	&output_memory_attribute.attr,
	&input_memory_attribute.attr,
	&output_frames_attribute.attr,
	&input_frames_attribute.attr,
	NULL,
};

struct attribute_group port_info_attr_group = {
	.name = "info",
	.attrs = info_attrs,
};






static ssize_t append_status_store(struct kobject *kobj, 
                                   struct kobj_attribute *attr, const char *buf, 
                                   size_t count)
{
	struct fscc_port *port = 0;
	unsigned value = 0;
    char *end = 0;
    
	port = (struct fscc_port *)dev_get_drvdata((struct device *)kobj);

	value = (unsigned)simple_strtoul(buf, &end, 16);

	fscc_port_set_append_status(port, value);

	return count;
}

static ssize_t append_status_show(struct kobject *kobj, 
                                  struct kobj_attribute *attr, char *buf)
{
	struct fscc_port *port = 0;
	
	port = (struct fscc_port *)dev_get_drvdata((struct device *)kobj);
	
	return sprintf(buf, "%i\n", fscc_port_get_append_status(port));
}

static struct kobj_attribute append_status_attribute = 
	__ATTR(append_status, SYSFS_READ_WRITE_MODE, append_status_show, append_status_store);

static struct attribute *settings_attrs[] = {
	&append_status_attribute.attr,
	NULL,
};

struct attribute_group port_settings_attr_group = {
	.name = "settings",
	.attrs = settings_attrs,
};






#ifdef DEBUG
static ssize_t interrupt_count(struct kobject *kobj, struct kobj_attribute *attr,
                               char *buf)

{
	struct fscc_port *port = 0;
	int interrupt_offset = 0;
	
	port = (struct fscc_port *)dev_get_drvdata((struct device *)kobj);

	interrupt_offset = str_to_interrupt_offset(attr->attr.name);
	
	if (interrupt_offset >= 0)
		return sprintf(buf, "%i\n", fscc_port_get_interrupt_count(port, interrupt_offset));

	return 0;
}
	
	
static struct kobj_attribute rfs_count_attribute = 
	__ATTR(rfs, SYSFS_READ_ONLY_MODE, interrupt_count, 0);	
	
static struct kobj_attribute rft_count_attribute = 
	__ATTR(rft, SYSFS_READ_ONLY_MODE, interrupt_count, 0);
	
static struct kobj_attribute rfe_count_attribute = 
	__ATTR(rfe, SYSFS_READ_ONLY_MODE, interrupt_count, 0);
	
static struct kobj_attribute rfo_count_attribute = 
	__ATTR(rfo, SYSFS_READ_ONLY_MODE, interrupt_count, 0);
	
static struct kobj_attribute rdo_count_attribute = 
	__ATTR(rdo, SYSFS_READ_ONLY_MODE, interrupt_count, 0);
	
static struct kobj_attribute rfl_count_attribute = 
	__ATTR(rfl, SYSFS_READ_ONLY_MODE, interrupt_count, 0);	
	
static struct kobj_attribute tin_count_attribute = 
	__ATTR(tin, SYSFS_READ_ONLY_MODE, interrupt_count, 0);
	
static struct kobj_attribute dr_hi_count_attribute = 
	__ATTR(dr_hi, SYSFS_READ_ONLY_MODE, interrupt_count, 0);
	
static struct kobj_attribute dt_hi_count_attribute = 
	__ATTR(dt_hi, SYSFS_READ_ONLY_MODE, interrupt_count, 0);
	
static struct kobj_attribute dr_fe_count_attribute = 
	__ATTR(dr_fe, SYSFS_READ_ONLY_MODE, interrupt_count, 0);
	
static struct kobj_attribute dt_fe_count_attribute = 
	__ATTR(dt_fe, SYSFS_READ_ONLY_MODE, interrupt_count, 0);
	
static struct kobj_attribute dr_stop_count_attribute = 
	__ATTR(dr_stop, SYSFS_READ_ONLY_MODE, interrupt_count, 0);
	
static struct kobj_attribute dt_stop_count_attribute = 
	__ATTR(dt_stop, SYSFS_READ_ONLY_MODE, interrupt_count, 0);
	
static struct kobj_attribute tft_count_attribute = 
	__ATTR(tft, SYSFS_READ_ONLY_MODE, interrupt_count, 0);
	
static struct kobj_attribute alls_count_attribute = 
	__ATTR(alls, SYSFS_READ_ONLY_MODE, interrupt_count, 0);
	
static struct kobj_attribute tdu_count_attribute = 
	__ATTR(tdu, SYSFS_READ_ONLY_MODE, interrupt_count, 0);	

static struct kobj_attribute ctss_count_attribute = 
	__ATTR(ctss, SYSFS_READ_ONLY_MODE, interrupt_count, 0);

static struct kobj_attribute dsrc_count_attribute = 
	__ATTR(dsrc, SYSFS_READ_ONLY_MODE, interrupt_count, 0);

static struct kobj_attribute cdc_count_attribute = 
	__ATTR(cdc, SYSFS_READ_ONLY_MODE, interrupt_count, 0);
	
static struct kobj_attribute ctsa_count_attribute = 
	__ATTR(ctsa, SYSFS_READ_ONLY_MODE, interrupt_count, 0);

static struct attribute *debug_attrs[] = {
	&rfs_count_attribute.attr,
	&rft_count_attribute.attr,
	&rfe_count_attribute.attr,
	&rfo_count_attribute.attr,
	&rdo_count_attribute.attr,
	&rfl_count_attribute.attr,
	
	&tin_count_attribute.attr,
	
	&dr_hi_count_attribute.attr,
	&dt_hi_count_attribute.attr,
	&dr_fe_count_attribute.attr,
	&dt_fe_count_attribute.attr,
	&dr_stop_count_attribute.attr,
	&dt_stop_count_attribute.attr,
	
	&tft_count_attribute.attr,
	&alls_count_attribute.attr,
	&tdu_count_attribute.attr,
	
	&ctss_count_attribute.attr,
	&dsrc_count_attribute.attr,
	&cdc_count_attribute.attr,
	&ctsa_count_attribute.attr,
	NULL,
};

struct attribute_group port_debug_attr_group = {
	.name = "debug",
	.attrs = debug_attrs,
};

#endif /* DEBUG */

#endif /* LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 25) */


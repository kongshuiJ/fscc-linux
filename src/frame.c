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

#include <linux/slab.h> /* kmalloc */

#include "frame.h"
#include "utils.h" /* return_{val_}if_true */
#include "port.h" /* struct fscc_port */
#include "card.h" /* struct fscc_card */

static unsigned frame_counter = 1;

void fscc_frame_update_buffer_size(struct fscc_frame *frame, unsigned length);

struct fscc_frame *fscc_frame_new(unsigned target_length, unsigned dma, struct fscc_port *port)
{
	struct fscc_frame *frame = 0;

	frame = kmalloc(sizeof(*frame), GFP_ATOMIC);

	return_val_if_untrue(frame, 0);

	INIT_LIST_HEAD(&frame->list);

	frame->target_length = 0;
	frame->current_length = 0;
	frame->data = 0;

	frame->dma = dma;
	frame->port = port;

	frame->number = frame_counter;
	frame_counter += 1;

	if (frame->dma) {
		frame->descriptor_handle = pci_map_single(frame->port->card->pci_dev,
				                                  &frame->descriptor,
				                                  sizeof(frame->descriptor),
					                              DMA_TO_DEVICE);
	}

	fscc_frame_update_buffer_size(frame, target_length);

	return frame;
}

void fscc_frame_delete(struct fscc_frame *frame)
{
	return_if_untrue(frame);

	if (frame->dma) {
		pci_unmap_single(frame->port->card->pci_dev, frame->descriptor_handle,
			             sizeof(frame->descriptor), DMA_TO_DEVICE);
	}

    if (frame->data)
		kfree(frame->data);

	kfree(frame);

	wake_up_interruptible(&output_queue);
}

unsigned fscc_frame_get_target_length(struct fscc_frame *frame)
{
	return_val_if_untrue(frame, 0);

	return frame->target_length;
}

unsigned fscc_frame_get_current_length(struct fscc_frame *frame)
{
	return_val_if_untrue(frame, 0);

	return frame->current_length;
}

unsigned fscc_frame_get_missing_length(struct fscc_frame *frame)
{
	return_val_if_untrue(frame, 0);

	return frame->target_length - frame->current_length;
}

unsigned fscc_frame_is_empty(struct fscc_frame *frame)
{
	return_val_if_untrue(frame, 0);

	return !fscc_frame_get_current_length(frame);
}

unsigned fscc_frame_is_full(struct fscc_frame *frame)
{
	return_val_if_untrue(frame, 0);

	return fscc_frame_get_current_length(frame) == fscc_frame_get_target_length(frame);
}

void fscc_frame_add_data(struct fscc_frame *frame, const char *data, unsigned length)
{
	return_if_untrue(frame);

	if (frame->dma && frame->data) {
		pci_unmap_single(frame->port->card->pci_dev, frame->data_handle,
			             frame->current_length, DMA_TO_DEVICE);
	}

	if (frame->current_length + length > frame->target_length)
		fscc_frame_update_buffer_size(frame, frame->current_length + length);

    memmove(frame->data + frame->current_length, data, length);
	frame->current_length += length;

	if (frame->dma && frame->data) {
		frame->data_handle = pci_map_single(frame->port->card->pci_dev,
		                                    frame->data,
		                                    frame->current_length,
		                                    DMA_TO_DEVICE);

		memset(&frame->descriptor, 0, sizeof(frame->descriptor));

		frame->descriptor.control = 0xA0000000 | frame->current_length;
		frame->descriptor.data_address = cpu_to_le32(frame->data_handle);
		frame->descriptor.data_count = frame->current_length;
		frame->descriptor.next_descriptor = 0;
	}
}

void fscc_frame_remove_data(struct fscc_frame *frame, unsigned length)
{
	return_if_untrue(frame);

	frame->current_length -= length;
}

char *fscc_frame_get_remaining_data(struct fscc_frame *frame)
{
	return_val_if_untrue(frame, 0);

	return frame->data + (frame->target_length - frame->current_length);
}

void fscc_frame_trim(struct fscc_frame *frame)
{
	return_if_untrue(frame);

	fscc_frame_update_buffer_size(frame, frame->current_length);
}

void fscc_frame_update_buffer_size(struct fscc_frame *frame, unsigned length)
{
	char *new_data = 0;

	return_if_untrue(frame);

	warn_if_untrue(length >= frame->current_length);

	if (length == 0) {
		if (frame->data) {
			kfree(frame->data);
			frame->data = 0;
		}

		frame->current_length = 0;
		frame->target_length = 0;

		return;
	}

	if (frame->target_length == length)
		return;

	new_data = kmalloc(length, GFP_KERNEL);

	return_if_untrue(new_data);

	memset(new_data, 0, sizeof(new_data));

	if (frame->data) {
		if (frame->current_length) //TODO: Is there a problem here if new_data is smaller than current_length?
			memmove(new_data, frame->data, frame->current_length);

		kfree(frame->data);
	}

	frame->data = new_data;
	frame->current_length = min(frame->current_length, length);
	frame->target_length = length;
}


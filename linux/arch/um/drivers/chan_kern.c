// $Id$
// $Locker$

// Author. Roar Thron�s.
// Modified User Mode Linux source file, 2001-2004.

/* 
 * Copyright (C) 2000 Jeff Dike (jdike@karaya.com)
 * Licensed under the GPL
 */

#include <linux/stddef.h>
#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/slab.h>
#include <linux/tty.h>
#include <linux/tty_flip.h>
#include <asm/irq.h>
#include "chan_kern.h"
#include "user_util.h"
#include "kern.h"
#include "irq_user.h"

static void tty_receive_char(struct tty_struct *tty, char ch)
{
	if(tty == NULL) return;

	if(I_IXON(tty) && !I_IXOFF(tty) && !tty->raw) {
		if(ch == STOP_CHAR(tty)){
			stop_tty(tty);
			return;
		}
		else if(ch == START_CHAR(tty)){
			start_tty(tty);
			return;
		}
	}

	if((tty->flip.flag_buf_ptr == NULL) || 
	   (tty->flip.char_buf_ptr == NULL))
		return;
	tty_insert_flip_char(tty, ch, TTY_NORMAL);
}

static int open_one_chan(struct chan *chan, int input, int output, int primary)
{
	int fd;

	if(chan->opened) return(0);
	fd = (*chan->ops->open)(input, output, primary, chan->data);
	if(fd < 0) return(fd);
	chan->fd = fd;

	chan->opened = 1;
	return(0);
}

int open_chan(struct list_head *chans)
{
	struct list_head *ele;
	struct chan *chan;
	int ret, err = 0;

	list_for_each(ele, chans){
		chan = list_entry(ele, struct chan, list);
		ret = open_one_chan(chan, chan->input, chan->output,
				    chan->primary);
		if(chan->primary) err = ret;
	}
	return(err);
}

void chan_enable_winch(struct list_head *chans, void *line)
{
	struct list_head *ele;
	struct chan *chan;

	list_for_each(ele, chans){
		chan = list_entry(ele, struct chan, list);
		if(chan->primary && chan->output && chan->ops->winch){
			register_winch(chan->fd, line);
			return;
		}
	}
}

void enable_chan(struct list_head *chans, 
		 int (*irq_setup)(int fd, int input, int output, void *data), 
		 void *data)
{
	struct list_head *ele;
	struct chan *chan;

	list_for_each(ele, chans){
		chan = list_entry(ele, struct chan, list);
		if(!chan->opened) continue;

		(*irq_setup)(chan->fd, chan->input, chan->output, data);
	}
}

void disable_chan(struct list_head *chans)
{
	struct list_head *ele;
	struct chan *chan;

	list_for_each(ele, chans){
		chan = list_entry(ele, struct chan, list);
		if(!chan->opened) continue;

		free_irq_by_fd(chan->fd);
	}
}

void close_chan(struct list_head *chans)
{
	struct list_head *ele;
	struct chan *chan;

	/* Close in reverse order as open in case more than one of them
	 * refers to the same device and they save and restore that device's
	 * state.  Then, the first one opened will have the original state,
	 * so it must be the last closed.
	 */
        for(ele = chans->prev; ele != chans; ele = ele->prev){
                chan = list_entry(ele, struct chan, list);
		if(chan->ops->close != NULL)
			(*chan->ops->close)(chan->fd, chan->data);
		free_irq_by_fd(chan->fd);
		chan->opened = 0;
	}
}

int write_chan(struct list_head *chans, const char *buf, int len)
{
	struct list_head *ele;
	struct chan *chan;
	int n, ret = 0;

	list_for_each(ele, chans){
		chan = list_entry(ele, struct chan, list);
		if(!chan->output) continue;
		n = chan->ops->write(chan->fd, buf, len, chan->data);
		if(chan->primary) ret = n;
	}
	return(ret);
}

int console_write_chan(struct list_head *chans, const char *buf, int len)
{
	struct list_head *ele;
	struct chan *chan;
	int n, ret = 0;

	list_for_each(ele, chans){
		chan = list_entry(ele, struct chan, list);
		if(!chan->output) continue;
		n = chan->ops->console_write(chan->fd, buf, len, chan->data);
		if(chan->primary) ret = n;
	}
	return(ret);
}

int chan_window_size(struct list_head *chans, unsigned short *rows_out,
		      unsigned short *cols_out)
{
	struct list_head *ele;
	struct chan *chan;

	list_for_each(ele, chans){
		chan = list_entry(ele, struct chan, list);
		if(chan->primary){
			if(chan->ops->window_size == NULL) return(0);
			return(chan->ops->window_size(chan->fd, chan->data,
						      rows_out, cols_out));
		}
	}
	return(0);
}

void free_one_chan(struct chan *chan)
{
	list_del(&chan->list);
	(*chan->ops->free)(chan->data);
	free_irq_by_fd(chan->fd);
	kfree(chan);
}

void free_chan(struct list_head *chans)
{
	struct list_head *ele, *next;
	struct chan *chan;

	list_for_each_safe(ele, next, chans){
		chan = list_entry(ele, struct chan, list);
		free_one_chan(chan);
	}
}

struct chan_type {
	char *key;
	struct chan_ops *ops;
};

struct chan_type chan_table[] = {
#ifdef CONFIG_FD_CHAN
	{ "fd", &fd_ops },
#endif
#ifdef CONFIG_NULL_CHAN
	{ "null", &null_ops },
#endif
#ifdef CONFIG_PORT_CHAN
	{ "port", &port_ops },
#endif
#ifdef CONFIG_PTY_CHAN
	{ "pty", &pty_ops },
	{ "pts", &pts_ops },
#endif
#ifdef CONFIG_TTY_CHAN
	{ "tty", &tty_ops },
#endif
#ifdef CONFIG_XTERM_CHAN
	{ "xterm", &xterm_ops },
#endif
};

static struct chan *parse_chan(char *str, int pri, int device, 
			       struct chan_opts *opts)
{
	struct chan_type *entry;
	struct chan_ops *ops;
	struct chan *chan;
	void *data;
	int i;

	ops = NULL;
	data = NULL;
	for(i = 0; i < sizeof(chan_table)/sizeof(chan_table[0]); i++){
		entry = &chan_table[i];
		if(!strncmp(str, entry->key, strlen(entry->key))){
			ops = entry->ops;
			str += strlen(entry->key);
			break;
		}
	}
	if(ops == NULL){
		printk(KERN_ERR "parse_chan couldn't parse \"%s\"\n", 
		       str);
		return(NULL);
	}
	data = (*ops->init)(str, device, opts);
	if(data == NULL) return(NULL);

	chan = kmalloc(sizeof(*chan), GFP_KERNEL);
	if(chan == NULL) return(NULL);
	*chan = ((struct chan) { list :	 	LIST_HEAD_INIT(chan->list),
				 primary :	1,
				 input :	0,
				 output :	0,
				 opened : 	0,
				 fd :		-1,
				 pri :		pri,
				 ops :		ops,
				 data :		data });
	return(chan);
}

int parse_chan_pair(char *str, struct list_head *chans, int pri, int device,
		    struct chan_opts *opts)
{
	struct chan *new, *chan;
	char *in, *out;

	if(!list_empty(chans)){
		chan = list_entry(chans->next, struct chan, list);
		if(chan->pri >= pri) return(0);
		free_chan(chans);
		INIT_LIST_HEAD(chans);
	}

	if((out = strchr(str, ',')) != NULL){
		in = str;
		*out = '\0';
		out++;
		new = parse_chan(in, pri, device, opts);
		if(new == NULL) return(-1);
		new->input = 1;
		list_add(&new->list, chans);

		new = parse_chan(out, pri, device, opts);
		if(new == NULL) return(-1);
		list_add(&new->list, chans);
		new->output = 1;
	}
	else {
		new = parse_chan(str, pri, device, opts);
		if(new == NULL) return(-1);
		list_add(&new->list, chans);
		new->input = 1;
		new->output = 1;
	}
	return(0);
}

int chan_out_fd(struct list_head *chans)
{
	struct list_head *ele;
	struct chan *chan;

	list_for_each(ele, chans){
		chan = list_entry(ele, struct chan, list);
		if(chan->primary && chan->output)
			return(chan->fd);
	}
	return(-1);
}

void chan_interrupt(struct list_head *chans, struct tty_struct *tty)
{
	struct list_head *ele, *next;
	struct chan *chan;
	char c;

	list_for_each_safe(ele, next, chans){
		chan = list_entry(ele, struct chan, list);
		if(!chan->input) continue;
		do {
			c = chan->ops->read(chan->fd, chan->data);
			if(c > 0) tty_receive_char(tty, c);
		} while(c > 0);
		if(c == 0) reactivate_fd(chan->fd);
		if(c == -EIO){
			if(chan->primary){
#ifndef CONFIG_VMS
				if(tty != NULL) tty_hangup(tty);
#endif
				close_chan(chans);
				free_chan(chans);
				return;
			}
			else {
				chan->ops->close(chan->fd, chan->data);
				free_one_chan(chan);
			}
		}
	}
#ifndef CONFIG_VMS
	if(tty) tty_flip_buffer_push(tty);
#endif
}

/*
 * Overrides for Emacs so that we follow Linus's tabbing style.
 * Emacs will notice this stuff at the end of the file and automatically
 * adjust the settings for this buffer only.  This must remain at the end
 * of the file.
 * ---------------------------------------------------------------------------
 * Local variables:
 * c-file-style: "linux"
 * End:
 */

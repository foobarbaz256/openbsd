/*	$OpenBSD: fdt.c,v 1.5 2016/04/03 12:32:13 patrick Exp $	*/

/*
 * Copyright (c) 2009 Dariusz Swiderski <sfires@sfires.net>
 * Copyright (c) 2009 Mark Kettenis <kettenis@sfires.net>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */


#include <sys/types.h>
#include <sys/param.h>
#include <sys/systm.h>

#include <dev/ofw/fdt.h>
#include <dev/ofw/openfirm.h>

unsigned int fdt_check_head(void *);
char	*fdt_get_str(u_int32_t);
void	*skip_property(u_int32_t *);
void	*skip_props(u_int32_t *);
void	*skip_node_name(u_int32_t *);
void	*skip_node(void *);
void	*fdt_parent_node_recurse(void *, void *);
int	 fdt_node_property_int(void *, char *, int *);
int	 fdt_node_property_ints(void *, char *, int *, int);
#ifdef DEBUG
void 	 fdt_print_node_recurse(void *, int);
#endif

static int tree_inited = 0;
static struct fdt tree;

unsigned int
fdt_check_head(void *fdt)
{
	struct fdt_head *fh;
	u_int32_t *ptr;

	fh = fdt;
	ptr = (u_int32_t *)fdt;

	if (betoh32(fh->fh_magic) != FDT_MAGIC)
		return 0;

	if (betoh32(fh->fh_version) > FDT_CODE_VERSION)
		return 0;

	if (betoh32(*(ptr + (betoh32(fh->fh_struct_off) / 4))) !=
	    FDT_NODE_BEGIN)
		return 0;

	/* check for end signature on version 17 blob */
	if ((betoh32(fh->fh_version) >= 17) &&
	    (betoh32(*(ptr + (betoh32(fh->fh_struct_off) / 4) +
	    (betoh32(fh->fh_struct_size) / 4) - 1)) != FDT_END))
		return 0;

	return betoh32(fh->fh_version);
}

/*
 * Initializes internal structures of module.
 * Has to be called once, preferably in machdep.c.
 */
int
fdt_init(void *fdt)
{
	int version;

	bzero(&tree, sizeof(struct fdt));
	tree_inited = 0;

	if (!fdt)
		return 0;

	if (!(version = fdt_check_head(fdt)))
		return 0;

	tree.header = (struct fdt_head *)fdt;
	tree.tree = (char *)fdt + betoh32(tree.header->fh_struct_off);
	tree.strings = (char *)fdt + betoh32(tree.header->fh_strings_off);
	tree.memory = (char *)fdt + betoh32(tree.header->fh_reserve_off);
	tree.version = version;
	tree.strings_size = betoh32(tree.header->fh_strings_size);
	tree_inited = 1;

	return version;
}

 /*
 * Return the size of the FDT.
 */
size_t
fdt_get_size(void *fdt)
{
	if (!fdt)
		return 0;

	if (!fdt_check_head(fdt))
		return 0;

	return betoh32(((struct fdt_head *)fdt)->fh_size);
}

/*
 * Retrieve string pointer from strings table.
 */
char *
fdt_get_str(u_int32_t num)
{
	if (num > tree.strings_size)
		return NULL;
	return (tree.strings) ? (tree.strings + num) : NULL;
}

/*
 * Utility functions for skipping parts of tree.
 */
void *
skip_property(u_int32_t *ptr)
{
	u_int32_t size;

	size = betoh32(*(ptr + 1));
	/* move forward by magic + size + nameid + rounded up property size */
	ptr += 3 + roundup(size, sizeof(u_int32_t)) / sizeof(u_int32_t);

	return ptr;
}

void *
skip_props(u_int32_t *ptr)
{
	while (betoh32(*ptr) == FDT_PROPERTY) {
		ptr = skip_property(ptr);
	}
	return ptr;
}

void *
skip_node_name(u_int32_t *ptr)
{
	/* skip name, aligned to 4 bytes, this is NULL term., so must add 1 */
	return ptr + roundup(strlen((char *)ptr) + 1,
	    sizeof(u_int32_t)) / sizeof(u_int32_t);
}

/*
 * Retrieves node property, the returned pointer is inside the fdt tree,
 * so we should not modify content pointed by it directly.
 */
int
fdt_node_property(void *node, char *name, char **out)
{
	u_int32_t *ptr;
	u_int32_t nameid;
	char *tmp;
	
	if (!tree_inited)
		return 0;

	ptr = (u_int32_t *)node;

	if (betoh32(*ptr) != FDT_NODE_BEGIN)
		return 0;

	ptr = skip_node_name(ptr + 1);

	while (betoh32(*ptr) == FDT_PROPERTY) {
		nameid = betoh32(*(ptr + 2)); /* id of name in strings table */
		tmp = fdt_get_str(nameid);
		if (!strcmp(name, tmp)) {
			*out = (char *)(ptr + 3); /* beginning of the value */
			return betoh32(*(ptr + 1)); /* size of value */
		}
		ptr = skip_property(ptr);
	}
	return 0;
}

/*
 * Retrieves next node, skipping all the children nodes of the pointed node,
 * returns pointer to next node, no matter if it exists or not.
 */
void *
skip_node(void *node)
{
	u_int32_t *ptr = node;

	ptr++;

	ptr = skip_node_name(ptr);
	ptr = skip_props(ptr);

	/* skip children */
	while (betoh32(*ptr) == FDT_NODE_BEGIN)
		ptr = skip_node(ptr);

	return (ptr + 1);
}

/*
 * Retrieves next node, skipping all the children nodes of the pointed node,
 * returns pointer to next node if exists, otherwise returns NULL.
 * If passed 0 will return first node of the tree (root).
 */
void *
fdt_next_node(void *node)
{
	u_int32_t *ptr;

	if (!tree_inited)
		return NULL;

	ptr = node;

	if (node == NULL) {
		ptr = tree.tree;
		return (betoh32(*ptr) == FDT_NODE_BEGIN) ? ptr : NULL;
	}

	if (betoh32(*ptr) != FDT_NODE_BEGIN)
		return NULL;

	ptr++;

	ptr = skip_node_name(ptr);
	ptr = skip_props(ptr);

	/* skip children */
	while (betoh32(*ptr) == FDT_NODE_BEGIN)
		ptr = skip_node(ptr);

	if (betoh32(*ptr) != FDT_NODE_END)
		return NULL;

	if (betoh32(*(ptr + 1)) != FDT_NODE_BEGIN)
		return NULL;

	return (ptr + 1);
}

/*
 * Retrieves node property as integers and puts them in the given
 * integer array.
 */
int
fdt_node_property_ints(void *node, char *name, int *out, int outlen)
{
	int *data;
	int i, inlen;

	inlen = fdt_node_property(node, name, (char **)&data) / sizeof(int);
	if (inlen <= 0)
		return -1;

	for (i = 0; i < inlen && i < outlen; i++)
		out[i] = betoh32(data[i]);

	return i;
}

/*
 * Retrieves node property as an integer.
 */
int
fdt_node_property_int(void *node, char *name, int *out)
{
	return fdt_node_property_ints(node, name, out, 1);
}

/*
 * Retrieves next node, skipping all the children nodes of the pointed node
 */
void *
fdt_child_node(void *node)
{
	u_int32_t *ptr;

	if (!tree_inited)
		return NULL;

	ptr = node;

	if (betoh32(*ptr) != FDT_NODE_BEGIN)
		return NULL;

	ptr++;

	ptr = skip_node_name(ptr);
	ptr = skip_props(ptr);
	/* check if there is a child node */
	return (betoh32(*ptr) == FDT_NODE_BEGIN) ? (ptr) : NULL;
}

/*
 * Retrieves node name.
 */
char *
fdt_node_name(void *node)
{
	u_int32_t *ptr;

	if (!tree_inited)
		return NULL;

	ptr = node;

	if (betoh32(*ptr) != FDT_NODE_BEGIN)
		return NULL;

	return (char *)(ptr + 1);
}

void *
fdt_find_node(char *name)
{
	void *node = fdt_next_node(0);
	const char *p = name;

	if (!tree_inited)
		return NULL;

	if (*p != '/')
		return NULL;

	while (*p) {
		void *child;
		const char *q;

		while (*p == '/')
			p++;
		if (*p == 0)
			return node;
		q = strchr(p, '/');
		if (q == NULL)
			q = p + strlen(p);

		for (child = fdt_child_node(node); child;
		     child = fdt_next_node(child)) {
			if (strncmp(p, fdt_node_name(child), q - p) == 0) {
				node = child;
				break;
			}
		}

		p = q;
	}

	return node;
}

void *
fdt_parent_node_recurse(void *pnode, void *child)
{
	void *node = fdt_child_node(pnode);
	void *tmp;

	while (node && (node != child)) {
		if ((tmp = fdt_parent_node_recurse(node, child)))
			return tmp;
		node = fdt_next_node(node);
	}
	return (node) ? pnode : NULL;
}

void *
fdt_parent_node(void *node)
{
	void *pnode = fdt_next_node(0);

	if (!tree_inited)
		return NULL;

	if (node == pnode)
		return NULL;

	return fdt_parent_node_recurse(pnode, node);
}

/*
 * Parse the memory address and size of a node.
 */
int
fdt_get_memory_address(void *node, int idx, struct fdt_memory *mem)
{
	void *parent;
	int ac, sc, off, ret, *in, inlen;

	if (node == NULL)
		return 1;

	parent = fdt_parent_node(node);
	if (parent == NULL)
		return 1;

	/* We only support 32-bit (1), and 64-bit (2) wide addresses here. */
	ret = fdt_node_property_int(parent, "#address-cells", &ac);
	if (ret != 1 || ac <= 0 || ac > 2)
		return 1;

	/* We only support 32-bit (1), and 64-bit (2) wide sizes here. */
	ret = fdt_node_property_int(parent, "#size-cells", &sc);
	if (ret != 1 || sc <= 0 || sc > 2)
		return 1;

	inlen = fdt_node_property(node, "reg", (char **)&in) / sizeof(int);
	if (inlen < ((idx + 1) * (ac + sc)))
		return 1;

	off = idx * (ac + sc);

	mem->addr = betoh32(in[off]);
	if (ac == 2)
		mem->addr = (mem->addr << 32) + betoh32(in[off + 1]);

	mem->size = betoh32(in[off + ac]);
	if (sc == 2)
		mem->size = (mem->size << 32) + betoh32(in[off + ac + 1]);

	/* TODO: translate memory address in ranges */
	//return fdt_translate_memory_address(parent, mem);

	return 0;
}

#ifdef DEBUG
/*
 * Debug methods for printing whole tree, particular odes and properies
 */
void *
fdt_print_property(void *node, int level)
{
	u_int32_t *ptr;
	char *tmp, *value;
	int cnt;
	u_int32_t nameid, size;

	ptr = (u_int32_t *)node;

	if (!tree_inited)
		return NULL;

	if (betoh32(*ptr) != FDT_PROPERTY)
		return ptr; /* should never happen */

	/* extract property name_id and size */
	size = betoh32(*++ptr);
	nameid = betoh32(*++ptr);

	for (cnt = 0; cnt < level; cnt++)
		printf("\t");

	tmp = fdt_get_str(nameid);
	printf("\t%s : ", tmp ? tmp : "NO_NAME");

	ptr++;
	value = (char *)ptr;

	if (!strcmp(tmp, "device_type") || !strcmp(tmp, "compatible") ||
	    !strcmp(tmp, "model") || !strcmp(tmp, "bootargs") ||
	    !strcmp(tmp, "linux,stdout-path")) {
		printf("%s", value);
	} else if (!strcmp(tmp, "clock-frequency") ||
	    !strcmp(tmp, "timebase-frequency")) {
		printf("%d", betoh32(*((unsigned int *)value)));
	} else {
		for (cnt = 0; cnt < size; cnt++) {
			if ((cnt % sizeof(u_int32_t)) == 0)
				printf(" ");
			printf("%02x", value[cnt]);
		}
	}
	ptr += roundup(size, sizeof(u_int32_t)) / sizeof(u_int32_t);
	printf("\n");

	return ptr;
}

void
fdt_print_node(void *node, int level)
{
	u_int32_t *ptr;
	int cnt;
	
	ptr = (u_int32_t *)node;

	if (betoh32(*ptr) != FDT_NODE_BEGIN)
		return;

	ptr++;

	for (cnt = 0; cnt < level; cnt++)
		printf("\t");
	printf("%s :\n", fdt_node_name(node));
	ptr = skip_node_name(ptr);

	while (betoh32(*ptr) == FDT_PROPERTY)
		ptr = fdt_print_property(ptr, level);
}

void
fdt_print_node_recurse(void *node, int level)
{
	void *child;

	fdt_print_node(node, level);
	for (child = fdt_child_node(node); child; child = fdt_next_node(child))
		fdt_print_node_recurse(child, level + 1);
}

void
fdt_print_tree(void)
{
	fdt_print_node_recurse(fdt_next_node(0), 0);
}
#endif

int
OF_peer(int handle)
{
	void *node = (char *)tree.header + handle;

	if (handle == 0)
		node = fdt_find_node("/");
	else
		node = fdt_next_node(node);
	return node ? ((char *)node - (char *)tree.header) : 0;
}

int
OF_child(int handle)
{
	void *node = (char *)tree.header + handle;

	node = fdt_child_node(node);
	return node ? ((char *)node - (char *)tree.header) : 0;
}

int
OF_parent(int handle)
{
	void *node = (char *)tree.header + handle;

	node = fdt_parent_node(node);
	return node ? ((char *)node - (char *)tree.header) : 0;
}

int
OF_finddevice(char *name)
{
	void *node;

	node = fdt_find_node(name);
	return node ? ((char *)node - (char *)tree.header) : -1;
}

int
OF_getproplen(int handle, char *prop)
{
	void *node = (char *)tree.header + handle;
	char *data;

	return fdt_node_property(node, prop, &data);
}

int
OF_getprop(int handle, char *prop, void *buf, int buflen)
{
	void *node = (char *)tree.header + handle;
	char *data;
	int len;

	len = fdt_node_property(node, prop, &data);

	/*
	 * The "name" property is optional since version 16 of the
	 * flattened device tree specification, so we synthesize one
	 * from the unit name of the node if it is missing.
	 */
	if (len == 0 && strcmp(prop, "name") == 0) {
		data = fdt_node_name(node);
		if (data) {
			len = strlcpy(buf, data, buflen);
			data = strchr(buf, '@');
			if (data)
				*data = 0;
			return len + 1;
		}
	}

	if (len > 0)
		memcpy(buf, data, min(len, buflen));
	return len;
}

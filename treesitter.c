/*
 * treesitter syntax highlighting support for qemacs.
 *
 * Copyright (c) 2022 Duncan Mac-Vicar P.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#include "qe.h"

#include <stdio.h>
#include <tree_sitter/api.h>

ModeDef treesitter_mode;

// FIXME this should go somewhere else?
TSLanguage *tree_sitter_json();
TSLanguage *tree_sitter_go();

#if 0
#ifdef DEBUG
#define dprintf(fmt, args...) fprintf(stderr, fmt, ##args)
#else
#define dprintf(fmt, args...)
#define NDEBUG
#endif
#define ddprintf(fmt, args...)
#else
#ifdef DEBUG
#define dprintf(fmt, ...)  fprintf(stderr, fmt, ##__VA_ARGS__)
#else
#define dprintf(...)
#define NDEBUG
#endif
#define ddprintf(...)
#endif

static int treesitter_mode_probe(ModeDef *mode, ModeProbeData *p)
{
    // always win
    return 1000000;
}

// implements TSInput read
static const char *ts_input_read(void *payload, uint32_t byte_index, TSPoint position, uint32_t *bytes_read) {
    EditBuffer *b = (EditBuffer *)payload;

    char *buf = qe_malloc_array(char, b->total_size);

    *bytes_read = eb_get_region_contents(b, byte_index, b->total_size,
                           buf, b->total_size);
    return buf;
}

typedef struct TreeSitterModeState {
    EditState *s;
    TSTree *tree;
    TSParser *parser;
} TreeSitterModeState;

// updates the treesitter AST as buffer edits are performed
static void treesitter_edit_buffer_callback(EditBuffer *b, void *opaque,
                                            int arg, enum LogOperation op,
                                            int offset, int size) {

    dprintf("treesitter_edit_buffer_callback: OP:%d OFFSET:%d SIZE:%d\n", op, offset, size);

    TreeSitterModeState *tsstate = (TreeSitterModeState *) opaque;
    TSTree *tree = (TSTree *) tsstate->tree;

    int line_num, col;

    TSInputEdit edit;
    edit.start_byte = offset;

    eb_get_pos(b, &line_num, &col, offset);
    edit.start_point = (TSPoint){.row = line_num, .column = col};

    switch (op) {
    case LOGOP_INSERT:
        edit.old_end_byte = offset;
        edit.new_end_byte = offset + size;

        eb_get_pos(b, &line_num, &col, offset);
        edit.old_end_point = (TSPoint){.row = line_num, .column = col};

        eb_get_pos(b, &line_num, &col, offset + size);
        edit.new_end_point = (TSPoint){.row = line_num, .column = col};
        break;
    case LOGOP_DELETE:
        edit.old_end_byte = offset;
        edit.new_end_byte = offset - size;

        eb_get_pos(b, &line_num, &col, offset);
        edit.old_end_point = (TSPoint){.row = line_num, .column = col};

        eb_get_pos(b, &line_num, &col, offset - size);
        edit.new_end_point = (TSPoint){.row = line_num, .column = col};
        break;
    default:
        put_error(tsstate->s, "LOGOP not implemented: %d", op);
    }
    ts_tree_edit(tree, &edit);
}

/* colorize nodes for a single line
   receives the root node, the original code point buffer, line length, file byte offset for the line
 */
void treesitter_mode_colorize_node_line(TSNode root, unsigned int *buf, int line_len, int line_num,
                                   int line_offset) {
    // do not botter with empty lines
    if (line_len == 0) {
        goto exit;
    }

    TSTreeCursor cursor = ts_tree_cursor_new(root);

    TSPoint line_start = {.row = line_num, .column = 0};
    int idx = ts_tree_cursor_goto_first_child_for_point(&cursor, line_start);
    // don't bother if no children
    if (idx < 0) {
        goto exit;
    }

    while (1) {
        TSNode node = ts_tree_cursor_current_node(&cursor);
        // skip literals
        if (!ts_node_is_named(node)) {
            goto exit;
        }

        // doc relative offset
        int start_byte = ts_node_start_byte(node);
        int end_byte = ts_node_end_byte(node);

        // restrict to inside of line
        start_byte = max(line_offset, start_byte) - line_offset;
        end_byte = min(end_byte, line_offset + line_len) - line_offset;

        dprintf("LINE_NUM:%d LINE_LEN::%d line_offset:%d | NODE idx:%d SB:%d EB:%d RANGE: %d-%d %s\n", line_num, line_len, line_offset, idx, ts_node_start_byte(node), ts_node_end_byte(node), start_byte, end_byte, ts_node_type(node));

        int style = -1;
        if (!strcmp(ts_node_type(node), "object")) {
            style = QE_STYLE_HTML_COMMENT;
        } else if (!strcmp(ts_node_type(node), "string_content")) {
            style = QE_STYLE_STRING;
        } else if (!strcmp(ts_node_type(node), "string")) {
            style = QE_STYLE_STRING;
        } else if (!strcmp(ts_node_type(node), "comment")) {
            style = QE_STYLE_COMMENT;
        } else if (!strcmp(ts_node_type(node), "identifier")) {
            style = QE_STYLE_VARIABLE;
        } else if (!strcmp(ts_node_type(node), "package_clause")) {
            style = QE_STYLE_KEYWORD;
        } else if (!strcmp(ts_node_type(node), "literal_value")) {
            style = QE_STYLE_NUMBER;
        } else if (!strcmp(ts_node_type(node), "import_declaration")) {
            style = QE_STYLE_KEYWORD;
        } else if (!strcmp(ts_node_type(node), "interpreted_string_literal")) {
            style = QE_STYLE_STRING;
        }

        if (style > 0) {
            dprintf("set_color(%d, %d, %s)\n", start_byte, end_byte, qe_styles[style].name);
            set_color(buf + start_byte, buf + end_byte, style);
        }

        treesitter_mode_colorize_node_line(node, buf, line_len, line_num, line_offset);

        TSNode ts_tree_cursor_current_node(const TSTreeCursor *);
        if (!ts_tree_cursor_goto_next_sibling(&cursor)) {
            break;
        } 
    }
 exit:
    //ts_tree_cursor_delete(&cursor);
}

int treesitter_get_colorized_line(EditState *s, unsigned int *buf, int buf_size,
                                  int *offsetp, int line_num) {
    // eb_get_line will set offset to next line, so save it
    int line_offset = *offsetp;

    TreeSitterModeState *tsstate = (TreeSitterModeState *) s->b->priv_data;
    int line_len = eb_get_line(s->b, buf, buf_size, offsetp);

    TSTree *tree = (TSTree *) tsstate->tree;
    TSNode root = ts_tree_root_node(tree);

    dprintf("treesitter_get_colorized_line: BUF:%d SIZE:%d OFFSETP:%d LINE:%d LINE_LEN:%d\n", buf, buf_size, line_offset, line_num, line_len);

    int bom = (line_len > 0 && buf[0] == 0xFEFF);
    treesitter_mode_colorize_node_line(root, buf + bom, line_len - bom, line_num, line_offset);

    return line_len;
}

static int treesitter_mode_init(EditState *s, ModeSavedData *saved_data)
{
    TSParser *parser = ts_parser_new();

    if (match_extension(s->b->filename, "c|h|C|H")) {
        s->mode_name = "treesitter/c";
        s->mode_flags = TREESITTER_C;
    } else if (match_extension(s->b->filename, "json")) {
        ts_parser_set_language(parser, tree_sitter_json());
        s->mode_name = "treesitter/json";
        s->mode_flags = TREESITTER_JSON;
    } else if (match_extension(s->b->filename, "go")) {
        ts_parser_set_language(parser, tree_sitter_go());
        s->mode_name = "treesitter/go";
        s->mode_flags = TREESITTER_GO;
    }

    TSInput input;
    input.encoding = TSInputEncodingUTF8;
    input.payload = s->b;
    input.read = ts_input_read;

    TSTree *tree = ts_parser_parse(parser, NULL, input);

    TreeSitterModeState *tsstate = qe_malloc(TreeSitterModeState);
    tsstate->s = s;
    tsstate->parser = parser;
    tsstate->tree = tree;

    // perhaps s->mode_data is better
    s->b->priv_data = tsstate;

    eb_add_callback(s->b, treesitter_edit_buffer_callback, tsstate, 0);
    s->get_colorized_line = treesitter_get_colorized_line;

    return 0;
}

static void treesitter_mode_close(EditState *e)
{
    TSParser *parser = (TSParser *)e->b->priv_data;
    ts_parser_delete(parser);
}

static int treesitter_init()
{
    //text_mode.mode_init(s, saved_data);

    memcpy(&treesitter_mode, &text_mode, sizeof(ModeDef));
    treesitter_mode.name = "treesitter";
    treesitter_mode.mode_probe = treesitter_mode_probe;
    treesitter_mode.mode_init = treesitter_mode_init;
    treesitter_mode.mode_close = treesitter_mode_close;

    // as we override treesitter_get_colorized_line, no need to set
    // colorize_func

    qe_register_mode(&treesitter_mode);

    return 0;
}

qe_module_init(treesitter_init);

#include <stdio.h>
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
#include "qe.h"

static int lua_plugins_init(void) {
    int error;
    // Create new Lua state and load the lua libraries
    lua_State *L = luaL_newstate();
    luaL_openlibs(L);
    luaL_dofile(L, "test.lua");
    lua_close(L);
    return 0;
}

qe_module_init(lua_plugins_init);


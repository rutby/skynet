#include "zlib.h"
#include <lauxlib.h>
#include <lua.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

static int lcompress(lua_State *L)
{
    size_t len1;
    size_t len2;
    const unsigned char *data = (unsigned char *)luaL_checklstring(L, 1, &len1); //要压缩的数据
    len2 = compressBound(len1);

    unsigned char *desbuffer = (unsigned char *)malloc(len2);
    if (Z_OK != compress(desbuffer, &len2, data, len1))
    {
        lua_pushnil(L);
        lua_pushstring(L, "memory not enough");
        return 2;
    }
    int *length = (int *)&desbuffer[len2];
    *length = len1;
    lua_pushlstring(L, (char *)desbuffer, len2);
    free(desbuffer);
    return 1;
}
static int luncompress(lua_State *L)
{
    size_t len1;
    size_t len2;
    const unsigned char *data = (unsigned char *)luaL_checklstring(L, 1, &len1); //要压缩的数据
    len2 = *((int *)&data[len1]);
    unsigned char *desbuffer = malloc(len2);
    if (Z_OK != uncompress(desbuffer, &len2, data, len1))
    {
        lua_pushnil(L);
        lua_pushstring(L, "memory not enough");
        return 2;
    }
    lua_pushlstring(L, (char *)desbuffer, len2);
    free(desbuffer);
    return 1;
}
static const struct luaL_Reg l_libzmethods[] = {
    {"lcompress", lcompress},
    {"luncompress", luncompress},
    {NULL, NULL},
};

int luaopen_zlib(lua_State *L)
{
    luaL_checkversion(L);
    luaL_newlib(L, l_libzmethods);
    return 1;
}

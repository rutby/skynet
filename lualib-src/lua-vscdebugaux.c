#define LUA_LIB
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#include "lstate.h"
#include "atomic.h"
#include <unistd.h>

static const int HOOKKEY = 0;

// 计算调用层级
static int get_call_level(lua_State *L) {
    int level = 0;
    CallInfo *ci = &L->base_ci;
    for (; ci && ci != L->ci; ci = ci->next) {
        level++;
    }
    return level;
}

/*
** Auxiliary function used by several library functions: check for
** an optional thread as function's first argument and set 'arg' with
** 1 if this argument is present (so that functions can skip it to
** access their other arguments)
*/
static lua_State *getthread (lua_State *L, int *arg) {
  if (lua_isthread(L, 1)) {
    *arg = 1;
    return lua_tothread(L, 1);
  }
  else {
    *arg = 0;
    return L;  /* function will operate over current thread */
  }
}

/*
** Call hook function registered at hook table for the current
** thread (if there is one)
*/
static void hookf (lua_State *L, lua_Debug *ar) {
  static const char *const hooknames[] =
    {"call", "return", "line", "count", "tail call"};
  lua_rawgetp(L, LUA_REGISTRYINDEX, &HOOKKEY);
  if (lua_isfunction(L, -1)) {
      lua_pushstring(L, hooknames[(int)ar->event]);  /* push event name */
      lua_getinfo(L, "nSl", ar);
      lua_pushstring(L, ar->source);
      lua_pushstring(L, ar->what);
      lua_pushstring(L, ar->name);
      lua_pushinteger(L, ar->currentline);
      if (ar->event == LUA_HOOKCALL || ar->event == LUA_HOOKTAILCALL || ar->event == LUA_HOOKRET)
        lua_pushinteger(L, get_call_level(L));
      else
        lua_pushnil(L);
      lua_call(L, 6, 1);  /* call hook function */
      int yield = lua_toboolean(L, -1);
      lua_pop(L,1);
      if (yield) {
         lua_yield(L, 0);
      }
  }
}

/*
** Convert a string mask (for 'sethook') into a bit mask
*/
static int makemask (const char *smask, int count) {
  int mask = 0;
  if (strchr(smask, 'c')) mask |= LUA_MASKCALL;
  if (strchr(smask, 'r')) mask |= LUA_MASKRET;
  if (strchr(smask, 'l')) mask |= LUA_MASKLINE;
  if (count > 0) mask |= LUA_MASKCOUNT;
  return mask;
}

static int sethook (lua_State *L) {
  int arg, mask, count;
  lua_Hook func;
  lua_State *L1 = getthread(L, &arg);
  if (lua_isnoneornil(L, arg+1)) {  /* no hook? */
    lua_sethook(L1, NULL, 0, 0);
  }
  else {
    const char *smask = luaL_checkstring(L, arg+2);
    count = (int)luaL_optinteger(L, arg + 3, 0);
    func = hookf; 
    mask = makemask(smask, count);
    luaL_checktype(L, arg+1, LUA_TFUNCTION);
    lua_pushvalue(L, arg+1);
    lua_rawsetp(L, LUA_REGISTRYINDEX, &HOOKKEY);
    lua_sethook(L1, func, mask, count);
  }
  return 0;
}

// () -> int
static lua_Integer cur_seq = 0;
static int nextseq(lua_State *L) {
    lua_Integer seq =  ATOM_INC(&cur_seq);
    lua_pushinteger(L, seq);
    return 1;
}

static const luaL_Reg l[] = {
    {"sethook", sethook},
    {"nextseq", nextseq},
    {NULL, NULL},
};

int luaopen_skynet_vscdebugaux(lua_State *L) {
    luaL_newlib(L, l);
    return 1;
}
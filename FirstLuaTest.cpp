// FirstLuaTest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "CommonLua.h"

#include <iostream>
#include <string>

using namespace std;

lua_State *current_lua_state;

const char* TYPE_LUA_ARRAY = "luaNumberArray";

typedef struct NumArray{
	int content[256];
	int size = 256;
} NumArray;

void print_in_lua(lua_State* L, string msg){
	lua_getglobal(L, "print");
	lua_pushstring(L, msg.c_str());
	int error = lua_pcall(L, 1, 0, 0);
	if (error){
		cerr << "lua print cause a error: " << error << endl;
	}
}

int newArray(lua_State *L){
	NumArray *array = (NumArray*)lua_newuserdata(L, sizeof(NumArray));
	array->size = 256;
	for (auto i = 0; i != array->size; i++){
		array->content[i] = 0;
	}

	// find out the metatable that created in the function `luaopen_array`
	// which content the method 'get' and 'set'.
	luaL_getmetatable(L, TYPE_LUA_ARRAY); // metatable's at the top
	lua_setmetatable(L, -2);

	return 1;
}

int setArray(lua_State *L){
	NumArray *array = (NumArray*)lua_touserdata(L, -3);
	int index = lua_tointeger(L, -2);
	int value = lua_tointeger(L, -1);
 	if (index >= (array->size - 1) || index < 0){
		print_in_lua(current_lua_state, "index out of bound");
		return 0;
	}
	array->content[index] = value;
	return 0;
}

int array2string(lua_State *L){
	NumArray *array = (NumArray*)lua_touserdata(L, -1);
	string ret = string("array content: ");
	int index = 0;
	for (auto value : array->content){
		if (value == 0){
			continue;
		}
		ret = ret + to_string(value) + ",\t";
		index++;
		if (index >= 15){
			ret = ret + "\n";
			index = 0;
		}
	}
	lua_pushstring(L, ret.c_str());
	return 1;
}

int getArray(lua_State *L){
	NumArray *array = (NumArray*)lua_touserdata(L, -2);
	int index = lua_tointeger(L, -1);
	int value = array->content[index];
	lua_pushinteger(L, value);
	return 1;
}

int size(lua_State *L){
	NumArray *array = (NumArray*)lua_touserdata(L, -1);
	lua_pushinteger(L, (int)(array->size));
	return 1;
}

static const struct luaL_reg arraylib_f[] = {
	{"new", newArray},
	{ nullptr, nullptr },
};

static const struct luaL_reg arraylib_m[] = {
	{ "set", setArray },
	{ "get", getArray },
	{ "size", size },
	{ "__tostring", array2string },
	{ nullptr, nullptr },
};

int luaopen_array(lua_State *L){
	luaL_newmetatable(L, TYPE_LUA_ARRAY);
	//lua_pushstring(L, "__index");
	//lua_pushvalue(L, -2); // stack: (top->) metatable, "__tostring", metatable
	//lua_settable(L, -3); // result: metatable.__index = metatable

	// stack: (top->) metatable
	luaL_openlib(L, nullptr, arraylib_m, 0); // register with the metatable at the stack top

	lua_pushstring(L, "__index"); // 
	lua_pushstring(L, "get"); // result: (top->) "get", "__index", metatable
	lua_gettable(L, -3); // result: top-> (the function)__index, "get", metatable
	lua_settable(L, -3); // result: metatable.get = metatable.__index
	// stack: (top->)metatable

	// to make __newindex = set
	lua_pushstring(L, "__newindex"); 
	lua_pushstring(L, "set"); 
	lua_gettable(L, -3);
	lua_settable(L, -3);

	luaL_openlib(L, "numarray", arraylib_f, 0);
	return 0;
}

int _tmain(int argc, _TCHAR* argv[])
{	
	lua_State *L = luaL_newstate();
	current_lua_state = L;
	luaL_openlibs(L);
	if (L == NULL) {
		return 0;
	}

	char lua_code_buff[50];
	int error;

	luaopen_array(L);

	luaL_loadfile(L, "HelloWorld.lua");
	error = lua_pcall(L, 0, 0, 0);
	cerr << error;

	lua_close(L);
	system("pause");
	return 0;
}




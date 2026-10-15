#include <windows.h>
#include <stdlib.h>
#include <string.h>

#include "lauxlib.h"
#include "lualib.h"

#include "lua_bind.h"
#include "lua_bind_joypad.h"
#include "lua_module.h"
#include "lua_script.h"

typedef struct LuaRunRequest
{
	char *source;
	char *chunkName;
} LuaRunRequest;

static lua_State *luaState;
static lua_State *luaCoroutine;
static int luaCoroutineRef = LUA_NOREF;
static LuaRunRequest *pendingRequest;
static LuaRunRequest *activeRequest;
static volatile LONG luaModuleState = LUA_MODULE_STATE_CLOSED;
static SRWLOCK requestLock = SRWLOCK_INIT;

void lua_module_pre_update(void)
{
	LuaJoypadAppplyState();
}

BOOL lua_module_get_current_script(char *buffer, DWORD bufferSize)
{
	LuaRunRequest *request;

	if (buffer == NULL || bufferSize == 0)
		return FALSE;

	AcquireSRWLockShared(&requestLock);
	request = activeRequest != NULL ? activeRequest : pendingRequest;
	lstrcpyn(buffer, request != NULL ? request->chunkName : "", (int)bufferSize);
	ReleaseSRWLockShared(&requestLock);
	return request != NULL;
}

LuaModuleState lua_module_get_state(void)
{
	return (LuaModuleState)InterlockedCompareExchange(&luaModuleState,
		LUA_MODULE_STATE_CLOSED, LUA_MODULE_STATE_CLOSED);
}

BOOL lua_module_should_stop(void)
{
	return lua_module_get_state() == LUA_MODULE_STATE_STOPPING;
}

static void LuaExecutionHook(lua_State *state, lua_Debug *debug)
{
	(void)debug;
	if (lua_module_get_state() == LUA_MODULE_STATE_ABORTING)
		luaL_error(state, "Script aborted.");
	lua_yield(state, 0);
}

static void LuaRunRequestFree(LuaRunRequest *request)
{
	if (request == NULL)
		return;
	free(request->source);
	free(request->chunkName);
	free(request);
	}

static void LuaModuleFinish(LuaScriptStatus result, const char *error)
{
	LuaRunRequest *request;

	if (error != NULL)
		kmod_lua_output("LUA", error);
	if (luaCoroutineRef != LUA_NOREF)
		luaL_unref(luaState, LUA_REGISTRYINDEX, luaCoroutineRef);
	luaCoroutineRef = LUA_NOREF;
	luaCoroutine = NULL;
	AcquireSRWLockExclusive(&requestLock);
	request = activeRequest;
	activeRequest = NULL;
	ReleaseSRWLockExclusive(&requestLock);
	LuaRunRequestFree(request);
	InterlockedExchange(&luaModuleState, LUA_MODULE_STATE_READY);
	lua_script_set_status(result);
}

BOOL lua_module_init(void)
{
	if (luaState != NULL)
		return TRUE;

	luaState = luaL_newstate();
	if (luaState == NULL)
	{
		lua_script_set_status(LUA_SCRIPT_STATUS_UNAVAILABLE);
		kmod_lua_output("LUA", "Unable to create Lua state.");
		return FALSE;
	}

	luaL_openlibs(luaState);
	lua_bind_register(luaState);
	InterlockedExchange(&luaModuleState, LUA_MODULE_STATE_READY);
	lua_script_set_status(LUA_SCRIPT_STATUS_IDLE);
	return TRUE;
}

void lua_module_reset(void)
{
	if (lua_module_get_state() == LUA_MODULE_STATE_CLOSED)
	{
		lua_module_init();
		return;
	}

	lua_module_close();
	lua_module_init();
}

void lua_module_close(void)
{
	LuaRunRequest *pending;
	LuaRunRequest *active;

	InterlockedExchange(&luaModuleState, LUA_MODULE_STATE_CLOSED);
	AcquireSRWLockExclusive(&requestLock);
	pending = pendingRequest;
	active = activeRequest;
	pendingRequest = NULL;
	activeRequest = NULL;
	ReleaseSRWLockExclusive(&requestLock);
	LuaRunRequestFree(pending);
	LuaRunRequestFree(active);
	if (luaState != NULL)
	{
		lua_close(luaState);
		luaState = NULL;
	}
	luaCoroutine = NULL;
	luaCoroutineRef = LUA_NOREF;
	InterlockedExchange(&luaModuleState, LUA_MODULE_STATE_CLOSED);
	lua_script_set_status(LUA_SCRIPT_STATUS_NO_ROM);
}

BOOL lua_module_run(const char *buff, const char *tag)
{
	LuaRunRequest *request;
	const char *effectiveChunkName;
	size_t sourceLength;
	size_t chunkNameLength;

	if (buff == NULL)
		return FALSE;

	sourceLength = strlen(buff);
	effectiveChunkName = tag != NULL && tag[0] != '\0' ? tag : "editor";
	chunkNameLength = strlen(effectiveChunkName);
	request = (LuaRunRequest *)malloc(sizeof(*request));
	if (request == NULL)
		return FALSE;
	request->source = (char *)malloc(sourceLength + 1);
	request->chunkName = (char *)malloc(chunkNameLength + 1);
	if (request->source == NULL || request->chunkName == NULL)
	{
		free(request->source);
		free(request->chunkName);
		free(request);
		return FALSE;
	}
	memcpy(request->source, buff, sourceLength + 1);
	memcpy(request->chunkName, effectiveChunkName, chunkNameLength + 1);

	AcquireSRWLockExclusive(&requestLock);
	if (lua_module_get_state() != LUA_MODULE_STATE_READY || luaState == NULL)
	{
		ReleaseSRWLockExclusive(&requestLock);
		LuaRunRequestFree(request);
		return FALSE;
	}
	pendingRequest = request;
	InterlockedExchange(&luaModuleState, LUA_MODULE_STATE_RUNNING);
	ReleaseSRWLockExclusive(&requestLock);
	lua_script_set_status(LUA_SCRIPT_STATUS_RUNNING);
	return TRUE;
}

BOOL lua_module_stop(void)
{
	if (InterlockedCompareExchange(&luaModuleState, LUA_MODULE_STATE_STOPPING,
		LUA_MODULE_STATE_RUNNING) != LUA_MODULE_STATE_RUNNING)
		return FALSE;

	lua_script_set_status(LUA_SCRIPT_STATUS_STOPPING);
	return TRUE;
}

BOOL lua_module_abort(void)
{
	LuaModuleState state;

	for (;;)
	{
		state = lua_module_get_state();
		if (state != LUA_MODULE_STATE_RUNNING && state != LUA_MODULE_STATE_STOPPING)
			return FALSE;
		if (InterlockedCompareExchange(&luaModuleState, LUA_MODULE_STATE_ABORTING,
			state) == state)
			break;
	}

	lua_script_set_status(LUA_SCRIPT_STATUS_ABORTING);
	return TRUE;
}

void lua_module_update(void)
{
	LuaRunRequest *request = NULL;
	LuaScriptStatus result;
	const char *error;
	int status;
	int resultCount;

	if (luaState == NULL)
		return;

	AcquireSRWLockExclusive(&requestLock);
	if (pendingRequest != NULL)
	{
		request = pendingRequest;
		pendingRequest = NULL;
		activeRequest = request;
	}
	ReleaseSRWLockExclusive(&requestLock);

	if (request != NULL)
	{
		luaCoroutine = lua_newthread(luaState);
		luaCoroutineRef = luaL_ref(luaState, LUA_REGISTRYINDEX);
		lua_sethook(luaCoroutine, LuaExecutionHook, LUA_MASKCOUNT, 1000);
		status = luaL_loadbufferx(luaCoroutine, request->source, strlen(request->source),
			request->chunkName, NULL);
		free(request->source);
		request->source = NULL;
		if (status != LUA_OK)
		{
			error = lua_tostring(luaCoroutine, -1);
			LuaModuleFinish(LUA_SCRIPT_STATUS_ERROR,
				error != NULL ? error : "Lua script could not be loaded.");
			return;
		}
	}

	if (luaCoroutine == NULL)
		return;

	status = lua_resume(luaCoroutine, luaState, 0, &resultCount);
	if (status == LUA_YIELD)
		return;

	if (status == LUA_OK && (lua_module_get_state() == LUA_MODULE_STATE_STOPPING ||
		lua_module_get_state() == LUA_MODULE_STATE_ABORTING))
		result = LUA_SCRIPT_STATUS_STOPPED;
	else if (status == LUA_OK)
		result = LUA_SCRIPT_STATUS_COMPLETED;
	else if (lua_module_get_state() == LUA_MODULE_STATE_STOPPING ||
		lua_module_get_state() == LUA_MODULE_STATE_ABORTING)
		result = LUA_SCRIPT_STATUS_STOPPED;
	else
	{
		result = LUA_SCRIPT_STATUS_ERROR;
		error = lua_tostring(luaCoroutine, -1);
		LuaModuleFinish(result, error != NULL ? error : "Lua execution failed.");
		return;
	}
	LuaModuleFinish(result, NULL);
}


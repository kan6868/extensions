// myextension.cpp
// Extension lib defines
#define LIB_NAME "BocoKiddo"
#define MODULE_NAME "bocokiddo"

#include <dmsdk/sdk.h>

static int GetWorldPosition(lua_State* L)
{
	using namespace dmVMath;

	dmGameObject::HInstance instance = dmScript::CheckGOInstance(L, 1);

	const Quat& rotation    = dmGameObject::GetWorldRotation(instance);

	const Point3& position  = dmGameObject::GetWorldPosition(instance);

	const Vector3& scale    = dmGameObject::GetWorldScale(instance);

	const Point3& localPosition  = dmGameObject::GetPosition(instance);

	const Quat& invQuat = Conjugate(rotation);
	
	Vector3 vecResult ;

	vecResult.setX(localPosition.getX() + position.getX());
	vecResult.setY(localPosition.getY() + position.getY());
	vecResult.setZ(localPosition.getZ() + position.getZ());
	
	vecResult = dmVMath::Rotate(invQuat, vecResult);

	vecResult.setX(vecResult.getX() / scale.getX());
	vecResult.setY(vecResult.getY() / scale.getY());
	vecResult.setZ(vecResult.getZ() / scale.getZ());
	
	dmScript::PushVector3(L, vecResult);
	
	return 1;
}

// Functions exposed to Lua
static const luaL_reg Module_methods[] =
{
	{"get_world_position", GetWorldPosition},
	{0, 0}
};
static void LuaInit(lua_State* L)
{
	int top = lua_gettop(L);

	// Register lua names
	luaL_register(L, MODULE_NAME, Module_methods);

	lua_pop(L, 1);
	assert(top == lua_gettop(L));
}

static dmExtension::Result AppInitializeMyExtension(dmExtension::AppParams* params)
{
	return dmExtension::RESULT_OK;
}

static dmExtension::Result InitializeMyExtension(dmExtension::Params* params)
{
	// Init Lua
	LuaInit(params->m_L);
	return dmExtension::RESULT_OK;
}

static dmExtension::Result AppFinalizeMyExtension(dmExtension::AppParams* params)
{
	return dmExtension::RESULT_OK;
}

static dmExtension::Result FinalizeMyExtension(dmExtension::Params* params)
{
	return dmExtension::RESULT_OK;
}

static dmExtension::Result OnUpdateMyExtension(dmExtension::Params* params)
{
	return dmExtension::RESULT_OK;
}

static void OnEventMyExtension(dmExtension::Params* params, const dmExtension::Event* event)
{
	switch(event->m_Event)
	{
		case dmExtension::EVENT_ID_ACTIVATEAPP:
		break;
		case dmExtension::EVENT_ID_DEACTIVATEAPP:
		break;
		case dmExtension::EVENT_ID_ICONIFYAPP:
		break;
		case dmExtension::EVENT_ID_DEICONIFYAPP:
		break;
		default:
		break;
	}
}

// Defold SDK uses a macro for setting up extension entry points:
//
// DM_DECLARE_EXTENSION(symbol, name, app_init, app_final, init, update, on_event, final)

// MyExtension is the C++ symbol that holds all relevant extension data.
// It must match the name field in the `ext.manifest`
DM_DECLARE_EXTENSION(BocoKiddo, LIB_NAME, AppInitializeMyExtension, AppFinalizeMyExtension, InitializeMyExtension, OnUpdateMyExtension, OnEventMyExtension, FinalizeMyExtension)
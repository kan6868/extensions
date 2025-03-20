// bococam.cpp
// Extension lib defines
#define LIB_NAME "BocoCamera"
#define MODULE_NAME "bococam"

#define DISPLAY_WIDTH 1280.0
#define DISPLAY_HEIGHT 720.0

#define HALF_MULTIPLIER 0.5

#include <dmsdk/sdk.h>

using namespace dmVMath;

// Structure to hold the current state of the camera system
struct State
{
    bool isActive = false;   // Indicates if the camera system is active
    bool isSuspend = false;  // Indicates if the camera system is suspended
};

// Structure to hold the camera properties and settings
struct Camera
{
    dmGameObject::HInstance mainCam;   // Handle to the main camera object
    dmGameObject::HInstance worldTarget; // Handle to the world target

    float windowWidth = (float)DISPLAY_WIDTH;  // Window width for rendering
    float windowHeight = (float)DISPLAY_HEIGHT; // Window height for rendering

    float halfWidth = windowWidth * HALF_MULTIPLIER; // Half of the window width
    float halfHeight = windowHeight * HALF_MULTIPLIER; // Half of the window height

    float zoom = 1.0f;            // Zoom level of the camera
    float aspect = 1.0f;          // Aspect ratio of the camera
    
    float invZoom = 1.0f / (zoom * aspect);  // Inverse of the zoom level

    unsigned int displayWidth = DISPLAY_WIDTH;  // Display width
    unsigned int displayHeight = DISPLAY_HEIGHT; // Display height
};

// Static global variables to hold camera and state data
static Camera g_Camera;
static State g_State;

/**
 * Resizes the camera's viewport and adjusts the world target's scale based on the window size.
 * 
 * @param width The new width of the window.
 * @param height The new height of the window.
 * 
 * This function recalculates the scaling factor for the camera to maintain its aspect ratio
 * and applies the necessary transformations to the world target object. The scale factor is 
 * adjusted to ensure the camera view fits within the new window size.
 */
static void Resize(const int& width, const int& height)
{
    // Calculate the scaling factors for both axes (X and Y)
    float displayScaleX = g_Camera.windowWidth / g_Camera.displayWidth;
    float displayScaleY = g_Camera.windowHeight / g_Camera.displayHeight;
    
    g_Camera.windowWidth = width;
    g_Camera.windowHeight = height;

    g_Camera.halfWidth = width * HALF_MULTIPLIER;
    g_Camera.halfHeight = height * HALF_MULTIPLIER;

    // Find the minimum scale factor between X and Y, and apply the world scaling
    g_Camera.aspect = fmin(displayScaleX, displayScaleY);
    
    float scaleValue = g_Camera.zoom * g_Camera.aspect;

    // Update the inverse zoom factor
    g_Camera.invZoom = 1.0f / scaleValue;
    
    // Log the calculated scale factor
    dmLogInfo("Scale: %f", scaleValue);

    // Set the position of the camera (shifted to center the view)
    dmGameObject::SetPosition(g_Camera.mainCam, Point3(width * -HALF_MULTIPLIER, height * -HALF_MULTIPLIER, 0.0f));

    // Apply the calculated scale to the world target
    dmGameObject::SetScale(g_Camera.worldTarget, Vector3(scaleValue));
}
/**
 * Remaps a value from one range to another.
 * @param value The value to remap.
 * @param low1 The low end of the value's current range.
 * @param high1 The high end of the value's current range.
 * @param low2 The low end of the value's target range.
 * @param high2 The high end of the value's target range.
 * @return The remapped value.
 */
static float remap(const float& value, const float& low1, const float& high1, const float& low2, const float& high2)
{
    return low2 + (value - low1) * (high2 - low2) / (high1 - low1);
}

/**
 * Initializes the camera system with the given camera and world target game objects, and sets the window size.
 * 
 * @param URL|ID cam The game object instance for the camera.
 * @param URL|ID world The game object instance for the world target.
 * @param number width The width of the window.
 * @param number height The height of the window.
 * 
 * @return 0 This function does not return any value.
 * 
 * This function is called to set up the camera system, assigning game objects to the camera and world target,
 * and updating the window dimensions. The camera's world scale is also set based on the world target's scale.
 * It then calls the `Resize` function to update the camera view.
 */
static int InitCamera(lua_State* L)
{
    // Get the game object instances for the camera and world target from the Lua stack
    dmGameObject::HInstance cam = dmScript::CheckGOInstance(L, 1);
    dmGameObject::HInstance world = dmScript::CheckGOInstance(L, 2);
    
    // Get the window width and height from the Lua stack
    g_Camera.windowWidth = lua_tonumber(L, 3);
    g_Camera.windowHeight = lua_tonumber(L, 4);

    // Log the initialized window size
    dmLogInfo("InitCamera: %f %f", g_Camera.windowWidth, g_Camera.windowHeight);
    
    // Set the camera as active and store the provided instances
    g_State.isActive = true;
    g_Camera.mainCam = cam;
    g_Camera.worldTarget = world;

    // Get the scale of the world target and use it as the world scale
    g_Camera.zoom = dmGameObject::GetScale(world).getX();
    // g_Camera.worldScale = g_Camera.initScale;
    // Resize the camera's viewport based on the initial window size
    Resize(g_Camera.windowWidth, g_Camera.windowHeight);
    
    return 0;
}

/**
 * Sets the zoom level of the camera and updates the world target's scale accordingly.
 * @param number zoom The new zoom level for the camera.
 * @return 0 This function does not return any value.
 */
static int Zoom(lua_State* L)
{
    float zoom = lua_tonumber(L, 1);
    g_Camera.zoom = zoom;
    
    dmGameObject::SetScale(g_Camera.worldTarget, Vector3(zoom));
    return 0;
}

/**
 * Releases the camera and world target game objects, and marks the camera system as inactive.
 * 
 * @return 0 This function does not return any value.
 * 
 * This function deactivates the camera system by clearing the `mainCam` and `worldTarget` references,
 * and updating the camera's state to inactive.
 */
// Function to release the camera and reset the state
static int ReleaseCamera(lua_State* L)
{
    // Reset the camera and world target instances
    g_Camera.mainCam = nullptr;
    g_Camera.worldTarget = nullptr;
    
    // Set the camera state to inactive
    g_State.isActive = false;

    return 0;
}

/**
 * Resizes the camera's window and updates the camera view accordingly.
 * 
 * @param number width The new width of the window.
 * @param number height The new height of the window.
 * 
 * @return 0 This function does not return any value.
 * 
 * This function updates the camera window dimensions and calls the `Resize` function to adjust the camera's
 * viewport and world target scaling based on the new window size.
 */
static int ResizeCamera(lua_State* L)
{
    // Get the new window size from the Lua stack
    g_Camera.windowWidth = lua_tonumber(L, 1);
    g_Camera.windowHeight = lua_tonumber(L, 2);
    
    // Resize the camera's viewport based on the new window size
    Resize(g_Camera.windowWidth, g_Camera.windowHeight);
    
    return 0;
}

static int ScreenToWorld(lua_State* L)
{
    // Check if the camera system is active
    if (!g_State.isActive)
    {
        // If inactive, return nil to the Lua stack
        lua_pushnil(L);
        return 1;
    }
    
    float invZoomHalfWidth = g_Camera.invZoom * g_Camera.halfWidth;
    float invZoomHalfHeight = g_Camera.invZoom * g_Camera.halfHeight;

    dmVMath::Vector3* out = dmScript::CheckVector3(L, 1);
    float screenX = out->getX();
    float screenY = out->getY();

    screenX = remap(screenX, -g_Camera.halfWidth, g_Camera.halfWidth, -invZoomHalfWidth, invZoomHalfWidth);
    screenY = remap(screenY, -g_Camera.halfHeight, g_Camera.halfHeight, -invZoomHalfHeight, invZoomHalfHeight);

    out->setX(screenX);
    out->setY(screenY);

    dmScript::PushVector3(L, *out);
    return 1;
}
/**
 * Retrieves the world position of a given game object by factoring in its local position, world position,
 * scale, and rotation.
 * @param URL|ID instance The game object instance for which the world position is requested.
 * @return 1 The function returns the calculated world position as a `Vector3` on the Lua stack.
 */
static int LocalToWorld(lua_State* L)
{
    // Check if the camera system is active
    if (!g_State.isActive)
    {
        // If inactive, return nil to the Lua stack
        lua_pushnil(L);
        return 1;
    }

    // Get the game object instance for which the world position is requested
    dmGameObject::HInstance instance = dmScript::CheckGOInstance(L, 1);

    // Retrieve the world rotation, world position, world scale, and local position of the object
    const Quat& rotation    = dmGameObject::GetWorldRotation(instance);
    const Point3& position  = dmGameObject::GetWorldPosition(instance);
    const Vector3& scale    = dmGameObject::GetWorldScale(instance);
    const Point3& localPosition  = dmGameObject::GetPosition(instance);

    Vector3 vecResult ;

    vecResult.setX(localPosition.getX() * scale.getX());
    vecResult.setY(localPosition.getY() * scale.getY());
    vecResult.setZ(localPosition.getZ() * scale.getZ());

    vecResult = dmVMath::Rotate(rotation, vecResult);

    vecResult.setX(vecResult.getX() + position.getX());
    vecResult.setY(vecResult.getY() + position.getY());
    vecResult.setZ(vecResult.getZ() + position.getZ());

    dmScript::PushVector3(L, vecResult);
    return 1;
}

/**
 * Retrieves the world position of a given game object by factoring in its local position, world position,
 * scale, and rotation.
 * 
 * @param URL|ID instance The game object instance for which the world position is requested.
 * 
 * @return 1 The function returns the calculated world position as a `Vector3` on the Lua stack.
 * 
 * This function computes the final world position of the object by adding the local position to the world position.
 * It then applies the inverse of the object's rotation and scales the result by the object's world scale.
 * This provides the accurate world position, which is then returned to the Lua script.
 */
static int WorldToLocal(lua_State* L)
{
    // Check if the camera system is active
    if (!g_State.isActive)
    {
        // If inactive, return nil to the Lua stack
        lua_pushnil(L);
        return 1;
    }

    // Get the game object instance for which the world position is requested
    dmGameObject::HInstance instance = dmScript::CheckGOInstance(L, 1);

    // Retrieve the world rotation, world position, world scale, and local position of the object
    const Quat& rotation    = dmGameObject::GetWorldRotation(instance);
    const Point3& position  = dmGameObject::GetWorldPosition(instance);
    const Vector3& scale    = dmGameObject::GetWorldScale(instance);
    const Point3& localPosition  = dmGameObject::GetPosition(instance);

    // Calculate the inverse of the rotation quaternion
    const Quat& invQuat = Conjugate(rotation);
    
    // Initialize a vector to hold the result
    Vector3 vecResult ;

    // Combine the local position and world position to get the final position in the world space
    vecResult.setX(localPosition.getX() + position.getX());
    vecResult.setY(localPosition.getY() + position.getY());
    vecResult.setZ(localPosition.getZ() + position.getZ());

    // Apply the inverse rotation to the position to get the position in the local space
    vecResult = dmVMath::Rotate(invQuat, vecResult);

    // Adjust the position based on the object's scale
    vecResult.setX(vecResult.getX() / scale.getX());
    vecResult.setY(vecResult.getY() / scale.getY());
    vecResult.setZ(vecResult.getZ() / scale.getZ());
    
    // Push the calculated world position as a vector onto the Lua stack
    dmScript::PushVector3(L, vecResult);
    
    return 1;
}

// Functions exposed to Lua
static const luaL_reg Module_methods[] =
{
    {"init_camera", InitCamera},
    {"local_to_world", LocalToWorld},
    {"resize", ResizeCamera},
    {"release_camera", ReleaseCamera},
    {"screen_to_world", ScreenToWorld},
    {"world_to_local", WorldToLocal},
    {"zoom", Zoom},
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
	g_Camera.displayWidth = dmConfigFile::GetInt(params->m_ConfigFile, "display.width", DISPLAY_WIDTH);
	g_Camera.displayHeight = dmConfigFile::GetInt(params->m_ConfigFile, "display.height", DISPLAY_HEIGHT);
	dmLogInfo("AppInitializeMyExtension: %d %d", g_Camera.displayWidth, g_Camera.displayHeight);

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
			dmLogInfo("OnEventMyExtension: EVENT_ID_ACTIVATEAPP");
		break;
		case dmExtension::EVENT_ID_DEACTIVATEAPP:
			g_State.isSuspend = true;
		break;
		case dmExtension::EVENT_ID_ICONIFYAPP:
			dmLogInfo("OnEventMyExtension: EVENT_ID_ICONIFYAPP");
		break;
		case dmExtension::EVENT_ID_DEICONIFYAPP:
			g_State.isSuspend = false;
		break;
		default:
		break;
	}
}

DM_DECLARE_EXTENSION(BocoCamera, LIB_NAME, AppInitializeMyExtension, AppFinalizeMyExtension, InitializeMyExtension, OnUpdateMyExtension, OnEventMyExtension, FinalizeMyExtension)

///////////////////////////////////////////////////////////////////////////////
// scenemanager.cpp
// ================
// This file contains the implementation of the `SceneManager` class, which is 
// responsible for managing the preparation and rendering of 3D scenes. It 
// handles textures, materials, lighting configurations, and object rendering.
//
// AUTHOR: Brian Battersby
// INSTITUTION: Southern New Hampshire University (SNHU)
// COURSE: CS-330 Computational Graphics and Visualization
//
// INITIAL VERSION: November 1, 2023
// LAST REVISED: December 1, 2024
//
// RESPONSIBILITIES:
// - Load, bind, and manage textures in OpenGL.
// - Define materials and lighting properties for 3D objects.
// - Manage transformations and shader configurations.
// - Render complex 3D scenes using basic meshes.
//
// NOTE: This implementation leverages external libraries like `stb_image` for 
// texture loading and GLM for matrix and vector operations.
///////////////////////////////////////////////////////////////////////////////

#include "SceneManager.h"

#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#endif

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/transform.hpp>

// declaration of global variables
namespace
{
	const char* g_ModelName = "model";
	const char* g_ColorValueName = "objectColor";
	const char* g_TextureValueName = "objectTexture";
	const char* g_UseTextureName = "bUseTexture";
	const char* g_UseLightingName = "bUseLighting";
}

/***********************************************************
 *  SceneManager()
 *
 *  The constructor for the class
 ***********************************************************/
SceneManager::SceneManager(ShaderManager* pShaderManager)
{
	m_pShaderManager = pShaderManager;
	m_basicMeshes = new ShapeMeshes();
}

/***********************************************************
 *  ~SceneManager()
 *
 *  The destructor for the class
 ***********************************************************/
SceneManager::~SceneManager()
{
	m_pShaderManager = NULL;
	delete m_basicMeshes;
	m_basicMeshes = NULL;
}

/***********************************************************
 *  CreateGLTexture()
 *
 *  This method is used for loading textures from image files,
 *  configuring the texture mapping parameters in OpenGL,
 *  generating the mipmaps, and loading the read texture into
 *  the next available texture slot in memory.
 ***********************************************************/
bool SceneManager::CreateGLTexture(const char* filename, std::string tag)
{
	int width = 0;
	int height = 0;
	int colorChannels = 0;
	GLuint textureID = 0;

	// indicate to always flip images vertically when loaded
	stbi_set_flip_vertically_on_load(true);

	// try to parse the image data from the specified image file
	unsigned char* image = stbi_load(
		filename,
		&width,
		&height,
		&colorChannels,
		0);

	// if the image was successfully read from the image file
	if (image)
	{
		std::cout << "Successfully loaded image:" << filename << ", width:" << width << ", height:" << height << ", channels:" << colorChannels << std::endl;

		glGenTextures(1, &textureID);
		glBindTexture(GL_TEXTURE_2D, textureID);

		// set the texture wrapping parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		// set texture filtering parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		// if the loaded image is in RGB format
		if (colorChannels == 3)
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
		// if the loaded image is in RGBA format - it supports transparency
		else if (colorChannels == 4)
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
		else
		{
			std::cout << "Not implemented to handle image with " << colorChannels << " channels" << std::endl;
			return false;
		}

		// generate the texture mipmaps for mapping textures to lower resolutions
		glGenerateMipmap(GL_TEXTURE_2D);

		// free the image data from local memory
		stbi_image_free(image);
		glBindTexture(GL_TEXTURE_2D, 0); // Unbind the texture

		// register the loaded texture and associate it with the special tag string
		m_textureIDs[m_loadedTextures].ID = textureID;
		m_textureIDs[m_loadedTextures].tag = tag;
		m_loadedTextures++;

		return true;
	}

	std::cout << "Could not load image:" << filename << std::endl;

	// Error loading the image
	return false;
}

/***********************************************************
 *  BindGLTextures()
 *
 *  This method is used for binding the loaded textures to
 *  OpenGL texture memory slots.  There are up to 16 slots.
 ***********************************************************/
void SceneManager::BindGLTextures()
{
	for (int i = 0; i < m_loadedTextures; i++)
	{
		// bind textures on corresponding texture units
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, m_textureIDs[i].ID);
	}
}

/***********************************************************
 *  DestroyGLTextures()
 *
 *  This method is used for freeing the memory in all the
 *  used texture memory slots.
 ***********************************************************/
void SceneManager::DestroyGLTextures()
{
	for (int i = 0; i < m_loadedTextures; i++)
	{
		glGenTextures(1, &m_textureIDs[i].ID);
	}
}

/***********************************************************
 *  FindTextureID()
 *
 *  This method is used for getting an ID for the previously
 *  loaded texture bitmap associated with the passed in tag.
 ***********************************************************/
int SceneManager::FindTextureID(std::string tag)
{
	int textureID = -1;
	int index = 0;
	bool bFound = false;

	while ((index < m_loadedTextures) && (bFound == false))
	{
		if (m_textureIDs[index].tag.compare(tag) == 0)
		{
			textureID = m_textureIDs[index].ID;
			bFound = true;
		}
		else
			index++;
	}

	return(textureID);
}

/***********************************************************
 *  FindTextureSlot()
 *
 *  This method is used for getting a slot index for the previously
 *  loaded texture bitmap associated with the passed in tag.
 ***********************************************************/
int SceneManager::FindTextureSlot(std::string tag)
{
	int textureSlot = -1;
	int index = 0;
	bool bFound = false;

	while ((index < m_loadedTextures) && (bFound == false))
	{
		if (m_textureIDs[index].tag.compare(tag) == 0)
		{
			textureSlot = index;
			bFound = true;
		}
		else
			index++;
	}

	return(textureSlot);
}

/***********************************************************
 *  FindMaterial()
 *
 *  This method is used for getting a material from the previously
 *  defined materials list that is associated with the passed in tag.
 ***********************************************************/
bool SceneManager::FindMaterial(std::string tag, OBJECT_MATERIAL& material)
{
	if (m_objectMaterials.size() == 0)
	{
		return(false);
	}

	int index = 0;
	bool bFound = false;
	while ((index < m_objectMaterials.size()) && (bFound == false))
	{
		if (m_objectMaterials[index].tag.compare(tag) == 0)
		{
			bFound = true;
			material.diffuseColor = m_objectMaterials[index].diffuseColor;
			material.specularColor = m_objectMaterials[index].specularColor;
			material.shininess = m_objectMaterials[index].shininess;
		}
		else
		{
			index++;
		}
	}

	return(true);
}

/***********************************************************
 *  SetTransformations()
 *
 *  This method is used for setting the transform buffer
 *  using the passed in transformation values.
 ***********************************************************/
void SceneManager::SetTransformations(
	glm::vec3 scaleXYZ,
	float XrotationDegrees,
	float YrotationDegrees,
	float ZrotationDegrees,
	glm::vec3 positionXYZ)
{
	// variables for this method
	glm::mat4 modelView;
	glm::mat4 scale;
	glm::mat4 rotationX;
	glm::mat4 rotationY;
	glm::mat4 rotationZ;
	glm::mat4 translation;

	// set the scale value in the transform buffer
	scale = glm::scale(scaleXYZ);
	// set the rotation values in the transform buffer
	rotationX = glm::rotate(glm::radians(XrotationDegrees), glm::vec3(1.0f, 0.0f, 0.0f));
	rotationY = glm::rotate(glm::radians(YrotationDegrees), glm::vec3(0.0f, 1.0f, 0.0f));
	rotationZ = glm::rotate(glm::radians(ZrotationDegrees), glm::vec3(0.0f, 0.0f, 1.0f));
	// set the translation value in the transform buffer
	translation = glm::translate(positionXYZ);

	modelView = translation * rotationZ * rotationY * rotationX * scale;

	if (NULL != m_pShaderManager)
	{
		m_pShaderManager->setMat4Value(g_ModelName, modelView);
	}
}

/***********************************************************
 *  SetShaderColor()
 *
 *  This method is used for setting the passed in color
 *  into the shader for the next draw command
 ***********************************************************/
void SceneManager::SetShaderColor(
	float redColorValue,
	float greenColorValue,
	float blueColorValue,
	float alphaValue)
{
	// variables for this method
	glm::vec4 currentColor;

	currentColor.r = redColorValue;
	currentColor.g = greenColorValue;
	currentColor.b = blueColorValue;
	currentColor.a = alphaValue;

	if (NULL != m_pShaderManager)
	{
		m_pShaderManager->setIntValue(g_UseTextureName, false);
		m_pShaderManager->setVec4Value(g_ColorValueName, currentColor);
	}
}

/***********************************************************
 *  SetShaderTexture()
 *
 *  This method is used for setting the texture data
 *  associated with the passed in ID into the shader.
 ***********************************************************/
void SceneManager::SetShaderTexture(
	std::string textureTag)
{
	if (NULL != m_pShaderManager)
	{
		m_pShaderManager->setIntValue(g_UseTextureName, true);

		int textureID = -1;
		textureID = FindTextureSlot(textureTag);
		m_pShaderManager->setSampler2DValue(g_TextureValueName, textureID);
	}
}

/***********************************************************
 *  SetTextureUVScale()
 *
 *  This method is used for setting the texture UV scale
 *  values into the shader.
 ***********************************************************/
void SceneManager::SetTextureUVScale(float u, float v)
{
	if (NULL != m_pShaderManager)
	{
		m_pShaderManager->setVec2Value("UVscale", glm::vec2(u, v));
	}
}

/***********************************************************
 *  SetShaderMaterial()
 *
 *  This method is used for passing the material values
 *  into the shader.
 ***********************************************************/
void SceneManager::SetShaderMaterial(
	std::string materialTag)
{
	if (m_objectMaterials.size() > 0)
	{
		OBJECT_MATERIAL material;
		bool bReturn = false;

		bReturn = FindMaterial(materialTag, material);
		if (bReturn == true)
		{
			m_pShaderManager->setVec3Value("material.diffuseColor", material.diffuseColor);
			m_pShaderManager->setVec3Value("material.specularColor", material.specularColor);
			m_pShaderManager->setFloatValue("material.shininess", material.shininess);
		}
	}
}

/**************************************************************/
/*** STUDENTS CAN MODIFY the code in the methods BELOW for  ***/
/*** preparing and rendering their own 3D replicated scenes.***/
/*** Please refer to the code in the OpenGL sample project  ***/
/*** for assistance.                                        ***/
/**************************************************************/


/***********************************************************
 *  LoadSceneTextures()
 *
 *  This method is used for preparing the 3D scene by loading
 *  the textures in memory to support the 3D scene
 *  rendering
 ***********************************************************/

void SceneManager::LoadSceneTextures()
{
	// Create textures from image file locations
	bool bReturn = false;

	// Load metal texture for the headphone stand pieces
	bReturn = CreateGLTexture(
		"textures/blackmetal.png",
		"stand_metal");

	// Load dark texture for the plastic headset frame and earcups
	bReturn = CreateGLTexture(
		"textures/blackplastic.png",
		"headset_plastic");

	// Load fabric texture for the ear pads and headband cushion
	bReturn = CreateGLTexture(
		"textures/blackfoam.png",
		"earpad_fabric");

	// Load floor texture for the scene plane
	bReturn = CreateGLTexture(
		"textures/greywood.png",
		"floor");

	// Load monitor screen texture
	bReturn = CreateGLTexture(
		"textures/desktopss.png",
		"desktop");

	// Load Nintendo Switch 2 screen texture
	bReturn = CreateGLTexture(
		"textures/switch2homescreen.jpg",
		"switchscreen");

	// Load iPad screen texture
	bReturn = CreateGLTexture(
		"textures/ipadhomescreen.jpg",
		"ipadscreen");

	// Load keyboard teture
	bReturn = CreateGLTexture(
		"textures/keyboardtexture001.png",
		"keyboard");
	

	// After texture image data loads,
	// loaded textures must be bound to texture slots
	BindGLTextures();
}
/***********************************************************
 *  DefineObjectMaterials()
 *
 *  This method is used for configuring material settings
 *  for objects within the 3D scene.
 ***********************************************************/

void SceneManager::DefineObjectMaterials()
{
	// Material for the textured wooden tabletop
	OBJECT_MATERIAL woodMaterial;
	woodMaterial.diffuseColor = glm::vec3(0.8f, 0.8f, 0.8f);
	woodMaterial.specularColor = glm::vec3(0.2f, 0.2f, 0.2f);
	woodMaterial.shininess = 8.0f;
	woodMaterial.tag = "wood";

	m_objectMaterials.push_back(woodMaterial);

	// Material for the black metal headphone stand
	OBJECT_MATERIAL metalMaterial;
	metalMaterial.diffuseColor = glm::vec3(0.7f, 0.7f, 0.7f);
	metalMaterial.specularColor = glm::vec3(0.8f, 0.8f, 0.8f);
	metalMaterial.shininess = 32.0f;
	metalMaterial.tag = "metal";

	m_objectMaterials.push_back(metalMaterial);

	// Material for plastic and padded headset components
	OBJECT_MATERIAL headsetMaterial;
	headsetMaterial.diffuseColor = glm::vec3(0.7f, 0.7f, 0.7f);
	headsetMaterial.specularColor = glm::vec3(0.25f, 0.25f, 0.25f);
	headsetMaterial.shininess = 12.0f;
	headsetMaterial.tag = "headset";

	m_objectMaterials.push_back(headsetMaterial);
}

/***********************************************************
 *  SetupSceneLights()
 *
 *  This method is called to add and configure light
 *  sources for the 3D scene.
 ***********************************************************/

void SceneManager::SetupSceneLights()
{
	// Enable custom lighting so the shader uses configured point lights
	m_pShaderManager->setBoolValue(g_UseLightingName, true);

	// Main overhead point light provides broad, neutral room lighting
	m_pShaderManager->setBoolValue("pointLights[0].bActive", true);
	m_pShaderManager->setVec3Value("pointLights[0].position", 1.5f, 8.0f, 4.5f);
	m_pShaderManager->setVec3Value("pointLights[0].ambient", 0.18f, 0.18f, 0.20f);
	m_pShaderManager->setVec3Value("pointLights[0].diffuse", 0.95f, 0.95f, 0.95f);
	m_pShaderManager->setVec3Value("pointLights[0].specular", 0.85f, 0.85f, 0.85f);

	// Secondary cool blue light adds subtle color similar to monitor/RGB lighting
	m_pShaderManager->setBoolValue("pointLights[1].bActive", true);
	m_pShaderManager->setVec3Value("pointLights[1].position", 4.5f, 3.0f, 1.0f);
	m_pShaderManager->setVec3Value("pointLights[1].ambient", 0.025f, 0.035f, 0.055f);
	m_pShaderManager->setVec3Value("pointLights[1].diffuse", 0.12f, 0.22f, 0.45f);
	m_pShaderManager->setVec3Value("pointLights[1].specular", 0.12f, 0.20f, 0.45f);
}

/***********************************************************
 *  PrepareScene()
 *
 *  This method is used for preparing the 3D scene by loading
 *  the shapes, textures in memory to support the 3D scene
 *  rendering
 ***********************************************************/
void SceneManager::PrepareScene()
{
	// only one instance of a particular mesh needs to be
	// loaded in memory no matter how many times it is drawn
	// in the rendered 3D scene

	// Load textures used in scene (use the above LoadSceneTextures() method)
	LoadSceneTextures();

	// Define material properties used by scene objects
	DefineObjectMaterials();
	// Configure light sources used by scene
	SetupSceneLights();
	m_basicMeshes->LoadPlaneMesh();
	m_basicMeshes->LoadCylinderMesh();
	m_basicMeshes->LoadBoxMesh();
	m_basicMeshes->LoadTorusMesh();
	m_basicMeshes->LoadSphereMesh();
}

/***********************************************************
 *  RenderScene()
 *
 *  This method is used for rendering the 3D scene by
 *  transforming and drawing the basic 3D shapes
 ***********************************************************/
void SceneManager::RenderScene()
{
	// declare the variables for the transformations
	glm::vec3 scaleXYZ;
	float XrotationDegrees = 0.0f;
	float YrotationDegrees = 0.0f;
	float ZrotationDegrees = 0.0f;
	glm::vec3 positionXYZ;

	// Offset used to move the full headphone stand assembly as one object group
	glm::vec3 headphoneStandOffset = glm::vec3(7.2f, 0.0f, -3.8f);

	// Offset used to move the full Nintendo Switch dock assembly as one object group
	glm::vec3 switchDockOffset = glm::vec3(-1.6f, 0.0f, -1.1f);

	// set the XYZ scale for the mesh
	scaleXYZ = glm::vec3(20.0f, 1.0f, 7.0f);

	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;

	positionXYZ = glm::vec3(0.0f, 0.0f, -1.5f);

	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	SetTextureUVScale(2.0f, 2.0f);
	SetShaderTexture("floor");
	SetShaderMaterial("wood");
	m_basicMeshes->DrawPlaneMesh();

	/*******************************************
 * Front desk edge
 *******************************************/

	scaleXYZ = glm::vec3(20.0f, 0.60f, 0.25f);
	positionXYZ = glm::vec3(0.0f, -0.30f, 5.5f);

	SetTransformations(
		scaleXYZ,
		0.0f,
		0.0f,
		0.0f,
		positionXYZ);

	SetShaderTexture("floor");
	SetShaderMaterial("wood");
	m_basicMeshes->DrawBoxMesh();

	/*******************************************
	 * Back wall, left and right walls
	 *******************************************/

	// Back wall
	scaleXYZ = glm::vec3(40.0f, 16.0f, 0.20f);
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;
	positionXYZ = glm::vec3(0.0f, 5.0f, -12.0f);

	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetShaderColor(0.55f, 0.65f, 0.80f, 1.0f);
	SetShaderMaterial("headset");
	m_basicMeshes->DrawBoxMesh();

	// Left wall
	scaleXYZ = glm::vec3(0.20f, 16.0f, 24.0f);
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;
	positionXYZ = glm::vec3(-20.0f, 5.0f, 0.0f);

	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetShaderColor(0.50f, 0.60f, 0.75f, 1.0f);
	SetShaderMaterial("headset");
	m_basicMeshes->DrawBoxMesh();

	// Right wall
	scaleXYZ = glm::vec3(0.20f, 16.0f, 24.0f);
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;
	positionXYZ = glm::vec3(20.0f, 5.0f, 0.0f);

	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetShaderColor(0.50f, 0.60f, 0.75f, 1.0f);
	SetShaderMaterial("headset");
	m_basicMeshes->DrawBoxMesh();


	/*******************************************
	 * LED Monitor & Stand
	 *******************************************/

	 /*** Draw monitor circular base ***/
	scaleXYZ = glm::vec3(1.85f, 0.08f, 1.20f);
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;
	positionXYZ = glm::vec3(0.8f, 0.12f, -4.1f);

	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetTextureUVScale(1.0f, 1.0f);
	SetShaderTexture("stand_metal");
	SetShaderMaterial("metal");
	m_basicMeshes->DrawCylinderMesh();

	/*** Draw monitor stand pole ***/
	scaleXYZ = glm::vec3(0.14f, 1.65f, 0.14f);
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;
	positionXYZ = glm::vec3(0.8f, 0.25f, -4.22f);

	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetTextureUVScale(1.0f, 1.0f);
	SetShaderTexture("stand_metal");
	SetShaderMaterial("metal");
	m_basicMeshes->DrawCylinderMesh();

	/*** Draw monitor body and bezel ***/
	scaleXYZ = glm::vec3(8.2f, 4.15f, 0.12f);
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;
	positionXYZ = glm::vec3(0.8f, 3.45f, -4.1f);

	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetTextureUVScale(1.0f, 1.0f);
	SetShaderTexture("headset_plastic");
	SetShaderMaterial("headset");
	m_basicMeshes->DrawBoxMesh();

	/*** Draw monitor screen surface ***/
	scaleXYZ = glm::vec3(7.65f, 3.70f, 0.03f);
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;
	positionXYZ = glm::vec3(0.8f, 3.45f, -3.98f);

	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetTextureUVScale(1.0f, 1.0f);
	SetShaderTexture("desktop");
	SetShaderMaterial("headset");
	m_basicMeshes->DrawBoxMesh();

	/*******************************************
 * Nintendo Switch & Dock Assembly
 *******************************************/

 /*** Draw Nintendo Switch dock body ***/
	scaleXYZ = glm::vec3(1.55f, 1.05f, 0.28f);
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;
	positionXYZ = glm::vec3(-3.0f, 0.65f, -2.7f) + switchDockOffset;

	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetTextureUVScale(1.0f, 1.0f);
	SetShaderTexture("headset_plastic");
	SetShaderMaterial("headset");
	m_basicMeshes->DrawBoxMesh();

	/*** Draw Nintendo Switch tablet body ***/
	scaleXYZ = glm::vec3(1.85f, 0.95f, 0.08f);
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;
	positionXYZ = glm::vec3(-3.0f, 1.05f, -2.48f) + switchDockOffset;

	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetTextureUVScale(1.0f, 1.0f);
	SetShaderTexture("headset_plastic");
	SetShaderMaterial("headset");
	m_basicMeshes->DrawBoxMesh();

	/*** Draw Nintendo Switch screen surface ***/
	scaleXYZ = glm::vec3(1.35f, 0.72f, 0.03f);
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;
	positionXYZ = glm::vec3(-3.0f, 1.05f, -2.42f) + switchDockOffset;

	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetTextureUVScale(1.0f, 1.0f);
	SetShaderTexture("switchscreen");
	SetShaderMaterial("headset");
	m_basicMeshes->DrawBoxMesh();

	/*** Draw left Joy-Con side panel ***/
	scaleXYZ = glm::vec3(0.22f, 0.95f, 0.09f);
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;
	positionXYZ = glm::vec3(-4.04f, 1.05f, -2.40f) + switchDockOffset;

	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetShaderColor(0.08f, 0.08f, 0.08f, 1.0f);
	SetShaderMaterial("headset");
	m_basicMeshes->DrawBoxMesh();

	/*** Draw right Joy-Con side panel ***/
	scaleXYZ = glm::vec3(0.22f, 0.95f, 0.09f);
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;
	positionXYZ = glm::vec3(-1.96f, 1.05f, -2.40f) + switchDockOffset;

	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetShaderColor(0.08f, 0.08f, 0.08f, 1.0f);
	SetShaderMaterial("headset");
	m_basicMeshes->DrawBoxMesh();

	/*******************************************
 * iPad and iPad Keyboard Case Assembly
 *******************************************/

 /*** Draw iPad keyboard case base ***/
	scaleXYZ = glm::vec3(2.1f, 0.08f, 1.15f);
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;
	positionXYZ = glm::vec3(6.75f, 0.12f, -0.2f);

	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetTextureUVScale(1.0f, 1.0f);
	SetShaderTexture("headset_plastic");
	SetShaderMaterial("headset");
	m_basicMeshes->DrawBoxMesh();

	/*** Draw iPad tablet body ***/
	scaleXYZ = glm::vec3(1.9f, 1.25f, 0.08f);
	XrotationDegrees = -18.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;
	positionXYZ = glm::vec3(6.75f, 0.72f, -0.65f);

	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetTextureUVScale(1.0f, 1.0f);
	SetShaderTexture("headset_plastic");
	SetShaderMaterial("headset");
	m_basicMeshes->DrawBoxMesh();

	/*** Draw iPad screen surface ***/
	scaleXYZ = glm::vec3(1.65f, 1.02f, 0.03f);
	XrotationDegrees = -18.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;
	positionXYZ = glm::vec3(6.75f, 0.72f, -0.58f);

	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetTextureUVScale(1.0f, 1.0f);
	SetShaderTexture("ipadscreen");
	SetShaderMaterial("headset");
	m_basicMeshes->DrawBoxMesh();

	/*** Draw iPad rear support / hinge ***/
	scaleXYZ = glm::vec3(1.7f, 0.08f, 0.12f);
	XrotationDegrees = -18.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;
	positionXYZ = glm::vec3(6.75f, 0.32f, -0.50f);

	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetTextureUVScale(1.0f, 1.0f);
	SetShaderTexture("headset_plastic");
	SetShaderMaterial("headset");
	m_basicMeshes->DrawBoxMesh();

	/*******************************************
 * Keyboard Assembly
 *******************************************/

 /*** Draw keyboard base ***/
	scaleXYZ = glm::vec3(5.5f, 0.10f, 1.50f);
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;
	positionXYZ = glm::vec3(0.8f, 0.12f, -1.25f);

	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetShaderColor(0.015f, 0.015f, 0.018f, 1.0f);
	SetShaderMaterial("headset");
	m_basicMeshes->DrawBoxMesh();

	/*** Draw keyboard textured key surface ***/
	scaleXYZ = glm::vec3(5.15f, 0.04f, 1.28f);
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;
	positionXYZ = glm::vec3(0.8f, 0.205f, -1.25f);

	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetTextureUVScale(1.0f, 1.0f);
	SetShaderTexture("keyboard");
	SetShaderMaterial("headset");
	m_basicMeshes->DrawBoxMesh();

	/*******************************************
* Mouse & Mouse Pad
*******************************************/

	/*** Draw mouse pad ***/
	scaleXYZ = glm::vec3(1.60f, 0.03f, 2.00f);
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;
	positionXYZ = glm::vec3(4.5f, 0.03f, -0.4f);

	SetTransformations(scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	SetShaderColor(0.05f, 0.05f, 0.05f, 1.0f);
	SetShaderMaterial("headset");
	m_basicMeshes->DrawBoxMesh();

	/*** Draw mouse body ***/
	scaleXYZ = glm::vec3(0.35f, 0.18f, 0.55f);
	XrotationDegrees = 0.0f;
	YrotationDegrees = 20.0f;
	ZrotationDegrees = 0.0f;
	positionXYZ = glm::vec3(4.65f, 0.18f, -0.4f);

	SetTransformations(scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	SetShaderTexture("headset_plastic");
	SetShaderMaterial("headset");
	m_basicMeshes->DrawSphereMesh();

	/*** Draw mouse wheel ***/
	scaleXYZ = glm::vec3(0.04f, 0.04f, 0.08f);
	XrotationDegrees = 90.0f;
	YrotationDegrees = 20.0f;
	ZrotationDegrees = 0.0f;
	positionXYZ = glm::vec3(4.65f, 0.25f, -0.38f);

	SetTransformations(scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	SetShaderColor(0.2f, 0.2f, 0.2f, 1.0f);
	SetShaderMaterial("metal");
	m_basicMeshes->DrawCylinderMesh();

	/*******************************************
 * Nintendo Switch Pro Controller Assembly
 *******************************************/

 /*** Draw controller main body ***/
	scaleXYZ = glm::vec3(0.90f, 0.16f, 0.42f);
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;
	positionXYZ = glm::vec3(-3.2f, 0.22f, -0.75f);

	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetTextureUVScale(1.0f, 1.0f);
	SetShaderTexture("headset_plastic");
	SetShaderMaterial("headset");
	m_basicMeshes->DrawSphereMesh();

	/*** Draw left controller grip ***/
	scaleXYZ = glm::vec3(0.28f, 0.16f, 0.45f);
	XrotationDegrees = 0.0f;
	YrotationDegrees = -12.0f;
	ZrotationDegrees = 0.0f;
	positionXYZ = glm::vec3(-3.75f, 0.15f, -0.55f);

	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetTextureUVScale(1.0f, 1.0f);
	SetShaderTexture("headset_plastic");
	SetShaderMaterial("headset");
	m_basicMeshes->DrawSphereMesh();

	/*** Draw right controller grip ***/
	scaleXYZ = glm::vec3(0.28f, 0.16f, 0.45f);
	XrotationDegrees = 0.0f;
	YrotationDegrees = 12.0f;
	ZrotationDegrees = 0.0f;
	positionXYZ = glm::vec3(-2.65f, 0.15f, -0.55f);

	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetTextureUVScale(1.0f, 1.0f);
	SetShaderTexture("headset_plastic");
	SetShaderMaterial("headset");
	m_basicMeshes->DrawSphereMesh();

	/*** Draw left analog stick ***/
	scaleXYZ = glm::vec3(0.10f, 0.04f, 0.10f);
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;
	positionXYZ = glm::vec3(-3.45f, 0.38f, -0.82f);

	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetShaderColor(0.02f, 0.02f, 0.02f, 1.0f);
	SetShaderMaterial("headset");
	m_basicMeshes->DrawCylinderMesh();

	/*** Draw right button cluster ***/
	scaleXYZ = glm::vec3(0.08f, 0.03f, 0.08f);
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;
	positionXYZ = glm::vec3(-2.95f, 0.38f, -0.82f);

	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetShaderColor(0.08f, 0.08f, 0.08f, 1.0f);
	SetShaderMaterial("headset");
	m_basicMeshes->DrawCylinderMesh();

	/*******************************************
	 * Headphone Stand and Headphones Assembly
	 *******************************************/

	 /*** Draw headphone stand base ***/
	scaleXYZ = glm::vec3(2.2f, 0.24f, 2.2f);
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;
	positionXYZ = glm::vec3(0.0f, 0.25f, 0.0f) + headphoneStandOffset;

	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetTextureUVScale(1.0f, 1.0f);
	SetShaderTexture("stand_metal");
	SetShaderMaterial("metal");
	m_basicMeshes->DrawCylinderMesh();

	/*** Draw headphone stand support pole ***/
	scaleXYZ = glm::vec3(0.26f, 3.0f, 0.25f);
	positionXYZ = glm::vec3(0.0f, 0.50f, 0.0f) + headphoneStandOffset;

	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetTextureUVScale(1.0f, 1.0f);
	SetShaderTexture("stand_metal");
	SetShaderMaterial("metal");
	m_basicMeshes->DrawCylinderMesh();

	/*** Draw headphone support tab ***/
	scaleXYZ = glm::vec3(0.8f, 0.12f, 0.4f);
	positionXYZ = glm::vec3(0.0f, 3.50f, 0.0f) + headphoneStandOffset;

	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetTextureUVScale(1.0f, 1.0f);
	SetShaderTexture("stand_metal");
	SetShaderMaterial("metal");
	m_basicMeshes->DrawBoxMesh();

	/*** Draw top headphone band ***/
	scaleXYZ = glm::vec3(2.45f, 0.18f, 0.25f);
	positionXYZ = glm::vec3(-0.06f, 3.50f, 0.0f) + headphoneStandOffset;

	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetTextureUVScale(1.0f, 1.0f);
	SetShaderTexture("headset_plastic");
	SetShaderMaterial("headset");
	m_basicMeshes->DrawBoxMesh();

	/*** Draw top headband cushion ***/
	scaleXYZ = glm::vec3(1.8f, 0.14f, 0.35f);
	positionXYZ = glm::vec3(-0.06f, 3.65f, 0.0f) + headphoneStandOffset;

	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetTextureUVScale(2.0f, 2.0f);
	SetShaderTexture("earpad_fabric");
	SetShaderMaterial("headset");
	m_basicMeshes->DrawBoxMesh();

	/*** Draw left headphone side band ***/
	scaleXYZ = glm::vec3(0.18f, 1.15f, 0.25f);
	ZrotationDegrees = -12.0f;
	positionXYZ = glm::vec3(-1.32f, 2.85f, 0.0f) + headphoneStandOffset;

	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetTextureUVScale(1.0f, 1.0f);
	SetShaderTexture("headset_plastic");
	SetShaderMaterial("headset");
	m_basicMeshes->DrawBoxMesh();

	/*** Draw right headphone side band ***/
	scaleXYZ = glm::vec3(0.18f, 1.15f, 0.25f);
	ZrotationDegrees = 12.0f;
	positionXYZ = glm::vec3(1.20f, 2.85f, 0.0f) + headphoneStandOffset;

	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetTextureUVScale(1.0f, 1.0f);
	SetShaderTexture("headset_plastic");
	SetShaderMaterial("headset");
	m_basicMeshes->DrawBoxMesh();

	ZrotationDegrees = 90.0f;

	/*** Draw left outer earcup ***/
	scaleXYZ = glm::vec3(0.82f, 0.36f, 1.10f);
	positionXYZ = glm::vec3(-1.18f, 2.05f, 0.0f) + headphoneStandOffset;

	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetTextureUVScale(1.0f, 1.0f);
	SetShaderTexture("headset_plastic");
	SetShaderMaterial("headset");
	m_basicMeshes->DrawCylinderMesh();

	/*** Draw right outer earcup ***/
	scaleXYZ = glm::vec3(0.82f, 0.36f, 1.10f);
	positionXYZ = glm::vec3(1.45f, 2.05f, 0.0f) + headphoneStandOffset;

	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetTextureUVScale(1.0f, 1.0f);
	SetShaderTexture("headset_plastic");
	SetShaderMaterial("headset");
	m_basicMeshes->DrawCylinderMesh();

	/*** Draw left inner ear pad ***/
	scaleXYZ = glm::vec3(0.72f, 0.42f, 0.96f);
	positionXYZ = glm::vec3(-0.96f, 2.05f, 0.0f) + headphoneStandOffset;

	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetTextureUVScale(2.0f, 2.0f);
	SetShaderTexture("earpad_fabric");
	SetShaderMaterial("headset");
	m_basicMeshes->DrawCylinderMesh();

	/*** Draw right inner ear pad ***/
	scaleXYZ = glm::vec3(0.72f, 0.42f, 0.96f);
	positionXYZ = glm::vec3(1.23f, 2.05f, 0.0f) + headphoneStandOffset;

	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetTextureUVScale(2.0f, 2.0f);
	SetShaderTexture("earpad_fabric");
	SetShaderMaterial("headset");
	m_basicMeshes->DrawCylinderMesh();

}

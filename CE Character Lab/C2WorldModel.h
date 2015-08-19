/*
 * This class SHOULD derive from AnimatableModel
 * and stores state information for world objects
 * like trees, grass, and buildings.
 *
 * A world model is more advanced than a car file as it carries distant bitmap,
 * bounding box, and a special form of animations and flags.
 *
 * Note that no stock C2 world objects actually have animation data.
 *
 * TODO: Move common code into AnimatableModel, and allow this to derive from it
*/

#ifndef __CE_Character_Lab__C2WorldModel__
#define __CE_Character_Lab__C2WorldModel__

#include <stdio.h>
#include <memory>
#include <array>
#include <vector>
#include <string>

#include <iostream>
#include <fstream>

#include "g_shared.h"

class C2Texture;
class C2Animation;
class C2Geometry;

#define objectPLACEWATER       1
#define objectPLACEGROUND      2
#define objectPLACEUSER        4
#define objectCIRCLE           8
#define objectBOUND            16
#define objectNOBMP            32
#define objectNOLIGHT          64
#define objectDEFLIGHT         128
#define objectGRNDLIGHT        256
#define objectNOSOFT           512
#define objectNOSOFT2          1024
#define objectANIMATED         0x80000000

class C2WorldModel
{
private:
  std::unique_ptr<C2Texture> m_far_texture; // used to render object as a 2d bitmap at a distance
  std::unique_ptr<C2Animation> m_animation; // apparently, world models only support 1 animation. TODO: allow unlimited animations
  std::unique_ptr<C2Geometry> m_geometry;
  
  std::string m_model_name;

  std::array<Vector3d, 4> m_far_vertices;
  std::array<TBound, 8> m_bounding_box;
  
  TObjInfo* m_old_object_info; // Easier to use old object for now
  
  void _generateBoundingBox(std::vector<TPoint3d>& vertex_data); // old method for bounding box. Move to geo when able

public:
  C2WorldModel(std::ifstream& instream); // construct object using file data
  ~C2WorldModel();
  
  TObjInfo* getObjectInfo(); // TODO: Make this unnecessary
};

#endif /* defined(__CE_Character_Lab__C2WorldModel__) */



/*
 
 typedef struct TagObject {
 TObjInfo info; - OK
 TBound   bound[8]; - OK
 TBMPModel bmpmodel; - OK
 TModel  *model; - OK
 TVTL    vtl; - OK
 } TObject;

 typedef struct _ObjInfo {
 int  Radius;
 int  YLo, YHi;
 int  linelenght, lintensity;
 int  circlerad, cintensity;
 int  flags;
 int  GrRad;
 int  DefLight;
 int  LastAniTime;
 float BoundR;
 BYTE res[16];
 } TObjInfo;
 
 
 typedef struct _TBound {
	float cx, cy, a, b,  y1, y2;
 } TBound;
 
 typedef struct _VTLdata {
 int aniKPS, FramesCount, AniTime;
 short int* aniData;
 } TVTL; // basically tAni sans the name
 
 typedef struct _TBMPModel {
 Vector3d  gVertex[4];
	WORD     *lpTexture;
 } TBMPModel;



*/
//
//  g_shared.h
//  CE Character Lab
//
//  Created by Tyler Minard on 8/11/15.
//  Copyright (c) 2015 Tyler Minard. All rights reserved.
//

#pragma once

#define csONWATER          0x00010000
#define HiColor(R,G,B) ( ((R)<<10) + ((G)<<5) + (B) )

enum class CEMapType { C2, C1 };

typedef unsigned short WORD;

typedef struct TagVector3d {
  float x,y,z;
} Vector3d;

typedef struct _Point3d {
  float x;
  float y;
  float z;
  short owner;
  short hide;
} TPoint3d;

typedef struct TagTEXTURE  {
  WORD DataA[128*128];
  WORD DataB[64*64];
  WORD DataC[32*32];
  WORD DataD[16*16];
  WORD SDataC[2][32*32];
  int mR, mG, mB;
} TEXTURE;

typedef struct _Face {
  int v1, v2, v3;
  int   tax, tbx, tcx, tay, tby, tcy;
  WORD Flags,DMask;
  int Distant, Next, group;
  char reserv[12];
} TFace;


typedef struct _Facef {
  int v1, v2, v3;
  float tax, tbx, tcx, tay, tby, tcy;
  WORD Flags,DMask;
  int Distant, Next, group;
  char reserv[12];
} TFacef;

typedef struct {
  int VCount, FCount, TextureSize, TextureHeight;
  TPoint3d gVertex[1024];
  union {
    TFace    gFace  [1024];
    TFacef   gFacef [1024];
  };
  WORD     *lpTexture, *lpTexture2, *lpTexture3;
  float    VLight[4][1024];
} TModel;

typedef struct _Animation {
  char aniName[32];
  int aniKPS, FramesCount, AniTime;
  short int* aniData;
} TAni;

typedef struct _SoundFX {
  int  length;
  short int* lpData;
} TSFX;

typedef struct _TCharacterInfo {
  char ModelName[32];
  int AniCount,SfxCount;
  TModel* mptr;
  TAni Animation[64];
  TSFX SoundFX[64];
  
  //cross-reference for animations and sounds
  int  Anifx[64];
  //-> Animation IDs...
  int DIE[10];
  int WALK[10];
  int SLEEP[10];
  int EAT[10];
  int IDLE[10];
  int ATTACK[10];
  int SLIDE[10];
  int RUN[10];
  int SWIM[10];
} TCharacterInfo;


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
  unsigned char res[16];
} TObjInfo;

typedef struct _TBMPModel {
  Vector3d  gVertex[4];
  WORD     *lpTexture;
} TBMPModel;

typedef struct _TBound {
  float cx, cy, a, b,  y1, y2;
  // y1 (add to object placement). The starting height of the bounding box in world space
  // y2 The ending height of the bounding box in world space
} TBound;

typedef struct _VTLdata {
  int aniKPS, FramesCount, AniTime;
  short int* aniData;
} TVTL;

typedef struct TagObject {
  TObjInfo info;
  TBound   bound[8];
  TBMPModel bmpmodel;
  TModel  *model;
  TVTL    vtl;
} TObject;

struct FogData {
  int32_t rgb;
  float altitude;
  int32_t danger;
  float transparency;
  float hlimit;
};

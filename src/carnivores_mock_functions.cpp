//
//  carnivores_methods_mock.c
//  CE Character Lab
//
//  Created by Tyler Minard on 8/11/15.
//  Copyright (c) 2015 Tyler Minard. All rights reserved.
//

#include <stdio.h>

#include "g_shared.h"

#include <math.h>
#include <cstdlib>
#include <iostream>
#include <string>

#ifndef M_PI
  #define M_PI 3.14
#endif

extern WORD  FadeTab[65][0x8000];

void CreateFadeTab()
{
  for (int l=0; l<64; l++)
    for (int c=0; c<0x8000; c++) {
      int R = (c >> 10);
      int G = (c >>  5) & 31;
      int B = c & 31;
      
      R = (int)((float)R * (l) / 60.f + (float)rand() *0.2f / RAND_MAX); if (R>31) R=31;
      G = (int)((float)G * (l) / 60.f + (float)rand() *0.2f / RAND_MAX); if (G>31) G=31;
      B = (int)((float)B * (l) / 60.f + (float)rand() *0.2f / RAND_MAX); if (B>31) B=31;
      FadeTab[l][c] = HiColor(R, G, B);
    }
}

void GenerateMipMap(WORD* A, WORD* D, int L)
{
  for (int y=0; y<L; y++)
    for (int x=0; x<L; x++) {
      int C1 = *(A + x*2 +   (y*2+0)*2*L);
      int C2 = *(A + x*2+1 + (y*2+0)*2*L);
      int C3 = *(A + x*2 +   (y*2+1)*2*L);
      int C4 = *(A + x*2+1 + (y*2+1)*2*L);
      //C4 = C1;
      /*
       if (L==64)
       C3=((C3 & 0x7bde) +  (C1 & 0x7bde))>>1;
       */
      int B = ( ((C1>>0) & 31) + ((C2>>0) & 31) + ((C3>>0) & 31) + ((C4>>0) & 31) +2 ) >> 2;
      int G = ( ((C1>>5) & 31) + ((C2>>5) & 31) + ((C3>>5) & 31) + ((C4>>5) & 31) +2 ) >> 2;
      int R = ( ((C1>>10) & 31) + ((C2>>10) & 31) + ((C3>>10) & 31) + ((C4>>10) & 31) +2 ) >> 2;
      *(D + x + y * L) = HiColor(R,G,B);
    }
}

void ApplyAlphaFlags(WORD* tptr, int cnt)
{
/*#ifdef _soft
#else
  for (int w=0; w<cnt; w++)
    *(tptr+w)|=0x8000;
#endif*/
}

void CalcMidColor(WORD* tptr, int l, int &mr, int &mg, int &mb)
{
  for (int w=0; w<l; w++) {
    WORD c = *(tptr + w);
    mb+=((c>> 0) & 31)*8;
    mg+=((c>> 5) & 31)*8;
    mr+=((c>>10) & 31)*8;
  }
  
  mr/=l; mg/=l; mb/=l;
}

int CheckPlaceCollision2(Vector3d &v, int wc)
{
  return 0;
}
int CheckPlaceCollisionP(Vector3d &v)
{
  return 0;
}

float GetLandUpH(float x, float y)
{
  return 1.f;
}

float GetLandH(float x, float y)
{
  return 1.f;
}

int siRand(int R)
{
  int ranNum = ((double)((rand() % RAND_MAX) * ((double)R*2+1)) / (double)RAND_MAX - R);
  return ranNum;
}

double CorrectedAlpha(double a, double b)
{
  double d = fabs(a-b);
  if (d<M_PI) return (a+b)/2;
  else d = (a+M_PI*2-b);
  
  if (d<0) d+=2*M_PI;
  if (d>2*M_PI) d-=2*M_PI;
  return d;
}

double FindVectorAlpha(double vx, double vy)
{
  double al = atan2f(-vy, -vx)+M_PI;
  if (al<0) al = 2.f*M_PI-al;
  if (al>2.f*M_PI) al = al-2.f*M_PI;
  return al;
}

void CorrectModel(TModel*)
{
  
}

void BrightenTexture(unsigned short*, int) {
  
}

void GenerateAlphaFlags(TModel*) {}
void GenerateModelMipMaps(TModel*);
void Console_PrintLogString(std::string msg) {
  std::cout << msg << std::endl;
}
void DATASHIFT(unsigned short*, int)
{
  
}

void GenerateModelMipMaps(TModel*)
{
  
}

int rRand(int r)
{
  if (!r) return 0;
  int res = rand() % (r+1);// / (RAND_MAX);
  //if (res > r) res = r;
  return res;
}

void DeltaFunc(double &a, double b, double d)
{
  if (b > a) {
    a+=d; if (a > b) a = b;
  } else {
    a-=d; if (a < b) a = b;
  }
}

double SGN(double f)
{
  if (f<0) return -1.f;
    else return  1.f;
}

float AngleDifference(float a, float b)
{
  a-=b;
  a = (float)fabs(a);
  if (a > M_PI) a = 2*M_PI - a;
  return a;
}

int CheckPlaceCollision(Vector3d &v, int wc, int mc)
{
  return 0;
}

#ifndef CJJIGGLE_CVJIGGLEDEFORMER_H
#define CJJIGGLE_CVJIGGLEDEFORMER_H

#include <maya/MArrayDataHandle.h>
#include <maya/MDataBlock.h>
#include <maya/MDataHandle.h>
#include <maya/MPlug.h> 
#include <maya/MPoint.h> 
#include <maya/MPointArray.h> 
#include <maya/MMatrix.h> 
#include <maya/MTypeId.h> 
#include <maya/MPlug.h>
#include <maya/MFloatArray.h>
#include <maya/MFloatVectorArray.h>
#include <maya/MDoubleArray.h>
#include <maya/MIntArray.h>
#include <maya/MVector.h>
#include <maya/MVectorArray.h>
#include <maya/MMatrix.h>
#include <maya/MGlobal.h>
#include <maya/MTime.h>

#include <maya/MItGeometry.h>

#include <maya/MPxDeformerNode.h>
#include <maya/MFnNumericAttribute.h>
#include <maya/MFnMatrixAttribute.h>
#include <maya/MFnCompoundAttribute.h>
#include <maya/MFnUnitAttribute.h>
#include <maya/MFnMesh.h>
#include <maya/MFnData.h>
#include <maya/MThreadPool.h>


#include <stdio.h>
#include <math.h>
#include <map>
#include <omp.h>



class cvJiggle : public MPxDeformerNode {
 public:
  cvJiggle();
  virtual ~cvJiggle(); 

  virtual MStatus deform(MDataBlock& data,
                         MItGeometry& iter,
                         const MMatrix& mat,
                         unsigned int mIndex);

  virtual MStatus setDependentsDirty(const MPlug& plug, MPlugArray& plugArray);
  static void* creator();
  static MStatus initialize();

  static  MTypeId id;
  static  MObject aTime;
  static  MObject aDirectionBias;
  static  MObject aNormalStrength;
  static  MObject aScale;
  static  MObject aMaxVelocity;
  static  MObject aStartFrame;
  static  MObject aDampingMagnitude;
  static  MObject aStiffnessMagnitude;
  static  MObject aJiggleMap;
  static  MObject aStiffnessMap;
  static  MObject aDampingMap;
  static  MObject aPerGeometry;
  static  MObject aWorldMatrix;

 private:
  MStatus JumpToElement(MArrayDataHandle& hArray, unsigned int index);
  MStatus GetInputMesh(MDataBlock& data, unsigned int geomIndex, MObject* oInputMesh);


  // Store everything per input geometry
  std::map<unsigned int, MTime> previousTime_;
  std::map<unsigned int, bool> initialized_;
  std::map<unsigned int, bool> dirtyMap_;
  std::map<unsigned int, MPointArray> previousPoints_;
  std::map<unsigned int, MPointArray> currentPoints_;
  std::map<unsigned int, MFloatArray> weights_;
  std::map<unsigned int, MFloatArray> jiggleMap_;
  std::map<unsigned int, MFloatArray> stiffnessMap_;
  std::map<unsigned int, MFloatArray> dampingMap_;
  std::map<unsigned int, MIntArray> membership_;
};

#endif

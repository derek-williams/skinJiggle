#ifndef CVJIGGLE_CVJIGGLECMD_H
#define CVJIGGLE_CVJIGGLECMD_H

#include <maya/MSelectionList.h>
#include <maya/MPointArray.h>
#include <maya/MDagPath.h>
#include <maya/MDagPathArray.h>
#include <maya/MPlug.h>
#include <maya/MPlugArray.h>
#include <maya/MIntArray.h>
#include <maya/MFloatArray.h>
#include <maya/MDoubleArray.h>
#include <maya/MString.h>
#include <maya/MStringArray.h>
#include <maya/MGlobal.h>
#include <maya/MSyntax.h>
#include <maya/MString.h>
#include <maya/MArgList.h>
#include <maya/MArgDatabase.h>
#include <maya/MObject.h>
#include <maya/MObjectArray.h>
#include <maya/MDGModifier.h>

#include <maya/MItDependencyGraph.h>
#include <maya/MItGeometry.h>
#include <maya/MItSelectionList.h>

#include <maya/MFnDagNode.h>
#include <maya/MFnDependencyNode.h>
#include <maya/MFnMesh.h>
#include <maya/MFnTransform.h>
#include <maya/MPxCommand.h>

#include <vector>
#include <sstream>


class cvJiggleCmd : public MPxCommand  {              
 public:                              
  cvJiggleCmd();
  virtual ~cvJiggleCmd();
  virtual MStatus doIt(const MArgList& argList);
  virtual MStatus redoIt();
  virtual MStatus undoIt();
  virtual bool isUndoable() const;
  static void* creator();
  static MSyntax newSyntax();

  static const char* kNameShort;
  static const char* kNameLong;
  static const char* kDampingShort;
  static const char* kDampingLong;
  static const char* kStiffnessShort;
  static const char* kStiffnessLong;

 private:
  MStatus GetShapeNode(MDagPath& path);
  MStatus GetJiggleDeformer(MDagPath& pathGeo, MObject& oDeformer);
  MDGModifier dgMod_;
  MString name_;
  MSelectionList selection_;
  float damping_;
  float stiffness_;
};  

#endif

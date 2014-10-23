#include "cvJiggleDeformer.h"

MTypeId cvJiggle::id(0x0011580A);
MObject cvJiggle::aPerGeometry;
MObject cvJiggle::aDirectionBias;
MObject cvJiggle::aNormalStrength;
MObject cvJiggle::aDampingMagnitude;
MObject cvJiggle::aStiffnessMagnitude;
MObject cvJiggle::aStiffnessMap;
MObject cvJiggle::aDampingMap;
MObject cvJiggle::aJiggleMap;
MObject cvJiggle::aTime;
MObject cvJiggle::aStartFrame;
MObject cvJiggle::aWorldMatrix;
MObject cvJiggle::aScale;
MObject cvJiggle::aMaxVelocity;


MStatus cvJiggle::initialize() {
  MFnMatrixAttribute      mAttr;
  MFnNumericAttribute     nAttr;
  MFnUnitAttribute        uAttr;
  MFnCompoundAttribute    cAttr;
  MStatus            status;

  aTime = uAttr.create("time", "time", MFnUnitAttribute::kTime, 0.0);
  addAttribute(aTime);
  attributeAffects(aTime, outputGeom);

  aStartFrame = nAttr.create("startFrame", "startFrame", MFnNumericData::kInt, 0, &status);
  nAttr.setKeyable(true); 
  addAttribute(aStartFrame);
  attributeAffects(aStartFrame, outputGeom);

  aScale = nAttr.create("scale", "scale", MFnNumericData::kFloat, 1.0, &status);
  nAttr.setKeyable(true);
  addAttribute(aScale);
  attributeAffects(aScale, outputGeom);

  aDirectionBias = nAttr.create("directionBias", "directionBias", MFnNumericData::kFloat, 0.0, &status);
  nAttr.setMin(-1.0);
  nAttr.setMax(1.0);
  nAttr.setKeyable(true);
  addAttribute(aDirectionBias);
  attributeAffects(aDirectionBias, outputGeom);

  aNormalStrength = nAttr.create("normalStrength", "normalStrength", MFnNumericData::kFloat, 1.0, &status);
  nAttr.setMin(0.0);
  nAttr.setMax(1.0);
  nAttr.setKeyable(true);
  addAttribute(aNormalStrength);
  attributeAffects(aNormalStrength, outputGeom);

  aMaxVelocity = nAttr.create("maxVelocity", "maxVelocity", MFnNumericData::kFloat, 1.0, &status);
  nAttr.setKeyable(true);
  addAttribute(aMaxVelocity);
  attributeAffects(aMaxVelocity, outputGeom);

  aStiffnessMagnitude = nAttr.create("stiffness", "stiffness", MFnNumericData::kFloat, 1.0, &status);
  nAttr.setMin(0.0);
  nAttr.setMax(1.0);
  nAttr.setKeyable(true);
  addAttribute(aStiffnessMagnitude);
  attributeAffects(aStiffnessMagnitude, outputGeom);

  aDampingMagnitude = nAttr.create("damping", "damping", MFnNumericData::kFloat, 1.0, &status);
  nAttr.setMin(0.0);
  nAttr.setMax(1.0);
  nAttr.setKeyable(true);
  addAttribute(aDampingMagnitude);
  attributeAffects(aDampingMagnitude, outputGeom);

  aWorldMatrix = mAttr.create("worldMatrix", "worldMatrix");

  aJiggleMap = nAttr.create("jiggleMap", "jiggleMap", MFnNumericData::kFloat, 0.0, &status);
  nAttr.setMin(0.0);
  nAttr.setMax(1.0);
  nAttr.setArray(true); 
  nAttr.setUsesArrayDataBuilder(true);

  aStiffnessMap = nAttr.create("stiffnessMap", "stiffnessMap", MFnNumericData::kFloat, 1.0, &status);
  nAttr.setMin(0.0);
  nAttr.setMax(1.0);
  nAttr.setArray(true); 
  nAttr.setUsesArrayDataBuilder(true);

  aDampingMap = nAttr.create("dampingMap", "dampingMap", MFnNumericData::kFloat, 1.0, &status);
  nAttr.setMin(0.0);
  nAttr.setMax(1.0);
  nAttr.setArray(true); 
  nAttr.setUsesArrayDataBuilder(true);

  aPerGeometry = cAttr.create("perGeometry", "perGeometry", &status);
  cAttr.setArray(true);
  cAttr.addChild(aWorldMatrix);
  cAttr.addChild(aJiggleMap);
  cAttr.addChild(aDampingMap);
  cAttr.addChild(aStiffnessMap);
  cAttr.setUsesArrayDataBuilder(true);
  addAttribute(aPerGeometry);
  attributeAffects(aWorldMatrix, outputGeom);
  attributeAffects(aJiggleMap, outputGeom);
  attributeAffects(aStiffnessMap, outputGeom);
  attributeAffects(aDampingMap, outputGeom);

  MGlobal::executeCommand("makePaintable -attrType multiFloat -sm deformer cvJiggle weights");
  MGlobal::executeCommand("makePaintable -attrType multiFloat -sm deformer cvJiggle jiggleMap");
  MGlobal::executeCommand("makePaintable -attrType multiFloat -sm deformer cvJiggle stiffnessMap");
  MGlobal::executeCommand("makePaintable -attrType multiFloat -sm deformer cvJiggle dampingMap");

  return MS::kSuccess;
}


cvJiggle::cvJiggle() {
}


cvJiggle::~cvJiggle() { 
}


void* cvJiggle::creator() { 
  return new cvJiggle();
}


MStatus cvJiggle::setDependentsDirty(const MPlug& plug, MPlugArray& plugArray) {
  if (plug == aJiggleMap || plug == aStiffnessMap || plug == aDampingMap || plug == weights) {
    unsigned int geomIndex = 0;
    if (plug.isArray()) {
      geomIndex = plug.parent().logicalIndex();
    } else {
      geomIndex = plug.array().parent().logicalIndex();
    }
    dirtyMap_[geomIndex] = true;
  }
  return MS::kSuccess;
}


MStatus cvJiggle::JumpToElement(MArrayDataHandle& hArray, unsigned int index) {
  MStatus status;
  status = hArray.jumpToElement(index);
  if (MFAIL(status)) {
    MArrayDataBuilder builder = hArray.builder(&status);
    CHECK_MSTATUS_AND_RETURN_IT(status);
    builder.addElement(index, &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);
    status = hArray.set(builder);
    CHECK_MSTATUS_AND_RETURN_IT(status);
    status = hArray.jumpToElement(index);
    CHECK_MSTATUS_AND_RETURN_IT(status);
  }
  return status;
}


/**
  Get the input mesh from the data block

  @param[in] data Data block.
  @param[in] geomIndex Geometry index.
  @param[out] oInputMesh Mesh MObject.
  @return MStatus
 */
MStatus cvJiggle::GetInputMesh(MDataBlock& data, unsigned int geomIndex, MObject* oInputMesh) {
  MStatus status;
  MArrayDataHandle hInput = data.outputArrayValue(input, &status);
  status = hInput.jumpToElement(geomIndex);
  CHECK_MSTATUS_AND_RETURN_IT(status);
  MDataHandle hInputGeom = hInput.outputValue().child(inputGeom);
  *oInputMesh = hInputGeom.asMesh();
  return status;
}


MStatus cvJiggle::deform(MDataBlock& data, 
                         MItGeometry& itGeo,
                         const MMatrix& localToWorldMatrix, 
                         unsigned int geomIndex) {
  MStatus status;
    
  // Get incoming attributes
  MTime currentTime = data.inputValue(aTime).asTime();
  MPointArray points;
  itGeo.allPositions(points);
  float env = data.inputValue(envelope).asFloat();
  float dampingMagnitude = data.inputValue(aDampingMagnitude).asFloat();
  float stiffnessMagnitude = data.inputValue(aStiffnessMagnitude).asFloat();
  float scale = data.inputValue(aScale).asFloat();
  float maxVelocity = data.inputValue(aMaxVelocity).asFloat() * scale;
  float directionBias = data.inputValue(aDirectionBias).asFloat();
  float normalStrength = data.inputValue(aNormalStrength).asFloat();
  MMatrix worldToLocalMatrix = localToWorldMatrix.inverse();
  int startFrame = data.inputValue(aStartFrame).asInt();

  MFloatVectorArray normals;
  if (directionBias != 0.0f || normalStrength < 1.0f) {
    MObject oInputMesh;
    status = GetInputMesh(data, geomIndex, &oInputMesh);
    CHECK_MSTATUS_AND_RETURN_IT(status);
    MFnMesh fnMesh(oInputMesh, &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);
    status = fnMesh.getVertexNormals(false, normals);
    CHECK_MSTATUS_AND_RETURN_IT(status);
  }

  MPointArray& currentPoints = currentPoints_[geomIndex];
  MPointArray& previousPoints = previousPoints_[geomIndex];
  MTime& previousTime = previousTime_[geomIndex];
  if (!initialized_[geomIndex]) {
    // Initialize the point states
    previousTime = currentTime;
    initialized_[geomIndex] = true;
    dirtyMap_[geomIndex] = true;
    currentPoints.setLength(itGeo.count());
    previousPoints.setLength(itGeo.count());
    for (unsigned int i = 0; i < points.length(); ++i) {
      currentPoints[i] = points[i] * localToWorldMatrix;
      previousPoints[i] = currentPoints[i];
    }
  }
    

  // Check if the timestep is just 1 frame since we want a stable simulation
  double timeDifference = currentTime.value() - previousTime.value();
  if (timeDifference > 1.0 || timeDifference < 0.0 || currentTime.value() <= startFrame) {
    initialized_[geomIndex] = false;
    previousTime = currentTime;
    return MS::kSuccess;
  }

  MArrayDataHandle hGeo = data.inputArrayValue(aPerGeometry);
  status = JumpToElement(hGeo, geomIndex);
  CHECK_MSTATUS_AND_RETURN_IT(status);
  // Get the input matrix to clean the trigger
  MDataHandle hPerGeometry = hGeo.inputValue();
  MMatrix matrix = hPerGeometry.child(aWorldMatrix).asMatrix();

  MFloatArray& weights = weights_[geomIndex];
  MFloatArray& jiggleMap = jiggleMap_[geomIndex];
  MFloatArray& stiffnessMap = stiffnessMap_[geomIndex];
  MFloatArray& dampingMap = dampingMap_[geomIndex];
  MIntArray& membership = membership_[geomIndex];
  // Get the paint maps
  if (dirtyMap_[geomIndex] || itGeo.count() != membership.length()) {
    jiggleMap.setLength(itGeo.count());
    stiffnessMap.setLength(itGeo.count());
    dampingMap.setLength(itGeo.count());
    weights.setLength(itGeo.count());
    membership.setLength(itGeo.count());
    MArrayDataHandle hJiggleMap = hPerGeometry.child(aJiggleMap);
    MArrayDataHandle hStiffnessMap = hPerGeometry.child(aStiffnessMap);
    MArrayDataHandle hDampingMap = hPerGeometry.child(aDampingMap);
    int ii = 0;
    for (itGeo.reset(); !itGeo.isDone(); itGeo.next(), ii++) {
      // Jiggle map
      status = JumpToElement(hJiggleMap, itGeo.index());
      CHECK_MSTATUS_AND_RETURN_IT(status);
      jiggleMap[ii] = hJiggleMap.inputValue().asFloat();

      // Stiffness map
      status = JumpToElement(hStiffnessMap, itGeo.index());
      CHECK_MSTATUS_AND_RETURN_IT(status);
      stiffnessMap[ii] = hStiffnessMap.inputValue().asFloat();

      // Damping map
      status = JumpToElement(hDampingMap, itGeo.index());
      CHECK_MSTATUS_AND_RETURN_IT(status);
      dampingMap[ii] = hDampingMap.inputValue().asFloat();

      // Weight map
      membership[ii] = itGeo.index();
      weights[ii] = weightValue(data, geomIndex, itGeo.index());
    }
    dirtyMap_[geomIndex] = false;
  }

  MPoint goal, newPos;
  MVector velocity, goalForce, displacement;
  float damping, stiffness;
  double normalDot;
  #pragma omp parallel for private(goal, damping, stiffness, velocity, newPos, \
                                   goalForce, displacement, normalDot)
  for (int i = 0; i < (int)points.length(); ++i) {
    // Calculate goal position
    goal = points[i] * localToWorldMatrix;

    // Calculate damping coefficient
    damping = dampingMagnitude * dampingMap[i];

    // Calculate stiffness coefficient
    stiffness = stiffnessMagnitude * stiffnessMap[i];

    // Offset the point by the velocity
    velocity = (currentPoints[i] - previousPoints[i]) * (1.0f - damping);
    newPos = currentPoints[i] + velocity;
    // Attract the point back to the goal
    goalForce = (goal - newPos) * stiffness;
    newPos += goalForce;

    // Clamp to the max displacement 
    displacement = newPos - goal;
    if (displacement.length() > maxVelocity) {
      displacement = displacement.normal() * maxVelocity;
      newPos = goal + displacement;
    }

    if (normalStrength < 1.0f) {
      // Project displacement to normal
      normalDot = displacement * normals[membership[i]];
      newPos -= normals[membership[i]] * normalDot * (1.0f - normalStrength);
    }

    if (directionBias > 0.0f) {
      normalDot = displacement.normal() * normals[membership[i]];
      if (normalDot < 0.0) {
        newPos += displacement * ((displacement * normals[membership[i]]) * directionBias);
      }
    } else if (directionBias < 0.0f) {
      normalDot = displacement.normal() * normals[membership[i]];
      if (normalDot > 0.0) {
        newPos += displacement * ((displacement * normals[membership[i]]) * directionBias);
      }
    }

    // Store the previous points
    previousPoints[i] = currentPoints[i];
    currentPoints[i] = newPos;

    // Multiply by weight map and envelope
    points[i] += ((newPos * worldToLocalMatrix) - points[i]) * weights[i] * env * jiggleMap[i];
    
  }

  itGeo.setAllPositions(points);
  previousTime = currentTime;

  return status;
}




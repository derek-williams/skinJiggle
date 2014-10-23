#include "cvJiggleCmd.h"
#include "cvJiggleDeformer.h"


const char* cvJiggleCmd::kNameShort = "-n";
const char* cvJiggleCmd::kNameLong = "-name";
const char* cvJiggleCmd::kDampingShort = "-d";
const char* cvJiggleCmd::kDampingLong = "-damping";
const char* cvJiggleCmd::kStiffnessShort = "-s";
const char* cvJiggleCmd::kStiffnessLong = "-stiffness";


cvJiggleCmd::cvJiggleCmd()
    : name_("cvJiggle#"),
      damping_(0.5f),
      stiffness_(0.5f) {
}


cvJiggleCmd::~cvJiggleCmd() {
}


MSyntax cvJiggleCmd::newSyntax() {
  MSyntax syntax;
  syntax.addFlag(kNameShort, kNameLong, MSyntax::kString);
  syntax.addFlag(kDampingShort, kDampingLong, MSyntax::kDouble);
  syntax.addFlag(kStiffnessShort, kStiffnessLong, MSyntax::kDouble);
  syntax.setObjectType(MSyntax::kSelectionList, 1, 255);
  syntax.useSelectionAsDefault(true);
  syntax.enableQuery(false);
  syntax.enableEdit(false);
  return syntax;
}


void* cvJiggleCmd::creator() {                                
  return new cvJiggleCmd;                    
}    


bool cvJiggleCmd::isUndoable() const {
  return true;
}


/**
  Gets the shape node of a dag path
 
  @param[in,out] path Path to a dag node that could be a transform or a shape.
  On return, the path will be to a shape node if one exists.
  @return MStatus.
 */
MStatus cvJiggleCmd::GetShapeNode(MDagPath& path) {
  MStatus status;

  if (path.apiType() == MFn::kMesh) {
    return MS::kSuccess;
  }

  unsigned int shape_count;
  status = path.numberOfShapesDirectlyBelow(shape_count);
  CHECK_MSTATUS_AND_RETURN_IT(status);

  for (unsigned int i = 0; i < shape_count; ++i) {
    status = path.extendToShapeDirectlyBelow(i);
    CHECK_MSTATUS_AND_RETURN_IT(status);

    if (!path.hasFn(MFn::kMesh)) {
      path.pop();
      continue;
    }

    MFnDagNode fnNode(path, &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);
    if (!fnNode.isIntermediateObject()) {
      return MS::kSuccess;
    }
    path.pop();
  }

  return MS::kFailure;
}


MStatus cvJiggleCmd::doIt(const MArgList& args) {
  MStatus status;

  MArgDatabase argData(syntax(), args);
  MString nameCmd;

  argData.getObjects(selection_);

  // Get any flag arguments
  if (argData.isFlagSet(kNameShort)) {
    name_ = argData.flagArgumentString(kNameShort, 0, &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);
  }
  if (argData.isFlagSet(kDampingShort)) {
    damping_ = (float)argData.flagArgumentDouble(kDampingShort, 0, &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);
  }
  if (argData.isFlagSet(kStiffnessShort)) {
    stiffness_ = (float)argData.flagArgumentDouble(kStiffnessShort, 0, &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);
  }

  // Build the deformer command string
  MItSelectionList itSel(selection_, MFn::kTransform);
  int ii = 0;
  MDagPath pathTransform;
  MObject component;
  std::stringstream ss;
  ss << "deformer -type cvJiggle -n \"" << name_.asChar() << "\"";
  for (; !itSel.isDone(); itSel.next(), ii++) {
    status = itSel.getDagPath(pathTransform, component);
    CHECK_MSTATUS_AND_RETURN_IT(status);
    status = GetShapeNode(pathTransform);
    if (MFAIL(status)) {
      MGlobal::displayError(pathTransform.partialPathName() + " has no shape.");
      return MS::kFailure;
    }

    ss << " " << pathTransform.partialPathName().asChar();
  }
  status = dgMod_.commandToExecute(ss.str().c_str());
  CHECK_MSTATUS_AND_RETURN_IT(status);

  return redoIt();
}


MStatus cvJiggleCmd::redoIt() {
  MStatus status;
  status = dgMod_.doIt();
  CHECK_MSTATUS_AND_RETURN_IT(status);
  // Get the deformer MObject
  MDagPath pathTransform;
  status = selection_.getDagPath(0, pathTransform);
  CHECK_MSTATUS_AND_RETURN_IT(status);
  MObject oDeformer;
  status =GetJiggleDeformer(pathTransform, oDeformer);
  CHECK_MSTATUS_AND_RETURN_IT(status);
  MFnDependencyNode fnNode(oDeformer, &status);
  CHECK_MSTATUS_AND_RETURN_IT(status);
  setResult(fnNode.name());

  // Create a dgMod to set the initial values and create the required connections
  MDGModifier dgMod;

  // Connect time
  MPlug plugInTime = fnNode.findPlug(cvJiggle::aTime, false, &status);
  CHECK_MSTATUS_AND_RETURN_IT(status);
  MSelectionList list;
  status = MGlobal::getSelectionListByName("time1", list);
  MObject oTime;
  list.getDependNode(0, oTime);
  MFnDependencyNode fnTime(oTime, &status);
  CHECK_MSTATUS_AND_RETURN_IT(status);
  MPlug plugOutTime = fnTime.findPlug("outTime", false, &status);
  CHECK_MSTATUS_AND_RETURN_IT(status);
  dgMod.connect(plugOutTime, plugInTime);

  // Set the stiffness and damping
  MPlug plugStiffness = fnNode.findPlug(cvJiggle::aStiffnessMagnitude, false, &status);
  CHECK_MSTATUS_AND_RETURN_IT(status);
  MPlug plugDamping = fnNode.findPlug(cvJiggle::aDampingMagnitude, false, &status);
  CHECK_MSTATUS_AND_RETURN_IT(status);
#if MAYA_API_VERSION >= 201200
  dgMod.newPlugValueFloat(plugStiffness, stiffness_);
  dgMod.newPlugValueFloat(plugDamping, damping_);
#else
  plugStiffness.setFloat(stiffness_);
  plugDamping.setFloat(damping_);
#endif

  MPlug plugInMatrix = fnNode.findPlug(cvJiggle::aWorldMatrix, false, &status);
  CHECK_MSTATUS_AND_RETURN_IT(status);

  MItSelectionList itSel(selection_, MFn::kTransform);
  int geomIndex = 0;
  MDagPath pathNode;
  MObject component;
  for (; !itSel.isDone(); itSel.next(), ++geomIndex) {
    status = plugInMatrix.selectAncestorLogicalIndex(geomIndex, cvJiggle::aPerGeometry);
    CHECK_MSTATUS_AND_RETURN_IT(status);
    // Connect the world matrix plug
    status = itSel.getDagPath(pathNode, component);
    CHECK_MSTATUS_AND_RETURN_IT(status);
    MFnDagNode fnTransform(pathTransform, &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);
    MPlug plugOutWorldMatrix = fnTransform.findPlug("worldMatrix", false, &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);
    plugOutWorldMatrix = plugOutWorldMatrix.elementByLogicalIndex(0, &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);

    dgMod.connect(plugOutWorldMatrix, plugInMatrix);
  }
  status = dgMod.doIt();
  CHECK_MSTATUS_AND_RETURN_IT(status);

  return MS::kSuccess;
}


// Get the first cvJiggle deformer in the history of pathGeo
MStatus cvJiggleCmd::GetJiggleDeformer(MDagPath& pathGeo, MObject& oDeformer) {
  MStatus status;
  status = GetShapeNode(pathGeo);
  CHECK_MSTATUS_AND_RETURN_IT(status);
  MObject oMesh = pathGeo.node();
  MItDependencyGraph itDG(oMesh, MFn::kInvalid,
                          MItDependencyGraph::kUpstream,
                          MItDependencyGraph::kDepthFirst,
                          MItDependencyGraph::kNodeLevel, &status);
  CHECK_MSTATUS_AND_RETURN_IT(status);
  for (; !itDG.isDone(); itDG.next()) {
    MFnDependencyNode fnCurrentNode(itDG.currentItem(), &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);
    if (fnCurrentNode.typeId() == cvJiggle::id) {
      oDeformer = itDG.currentItem();
      return MS::kSuccess;
    }
  }
  std::stringstream ss;
  ss << "No cvJiggle deformer was found in the history of " << pathGeo.partialPathName().asChar();
  MGlobal::displayError(ss.str().c_str());
  return MS::kFailure;
}


MStatus cvJiggleCmd::undoIt() {
  MStatus status;
  return dgMod_.undoIt();
}

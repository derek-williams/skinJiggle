#include "cvJiggleDeformer.h"
#include "cvJiggleCmd.h"
#include <maya/MFnPlugin.h>


MStatus initializePlugin(MObject obj) { 
  MStatus status;  
  MFnPlugin fnPlugin(obj, "Derek Williams", "1.1", "Any");

  status = fnPlugin.registerNode("cvJiggle", cvJiggle::id, cvJiggle::creator,
                                 cvJiggle::initialize, MPxNode::kDeformerNode);
  CHECK_MSTATUS_AND_RETURN_IT(status);

  status = fnPlugin.registerCommand("cvJiggle",          
                                    cvJiggleCmd::creator,
                                    cvJiggleCmd::newSyntax);      
  CHECK_MSTATUS_AND_RETURN_IT(status);

  return status;
}


MStatus uninitializePlugin(MObject obj) {
  MStatus   status;
  MFnPlugin fnPlugin(obj);
  status = fnPlugin.deregisterCommand("cvJiggle");        
  CHECK_MSTATUS_AND_RETURN_IT(status);

  status = fnPlugin.deregisterNode(cvJiggle::id);
  CHECK_MSTATUS_AND_RETURN_IT(status);

  return status;
}

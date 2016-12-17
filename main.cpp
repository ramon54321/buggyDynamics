#include <iostream>
#include <fstream>
#include <istream>
#include <string>
#include <iomanip>
#include <ode/ode.h>
#include "GL/glew.h"
#include "GLFW/glfw3.h"

#include "glm/vec3.hpp" // glm::vec3
#include "glm/vec4.hpp" // glm::vec4
#include "glm/mat4x4.hpp" // glm::mat4
#include "glm/gtc/matrix_transform.hpp" // glm::translate, glm::rotate, glm::scale, glm::perspective
#include "glm/gtc/quaternion.hpp"
#include "glm/gtx/quaternion.hpp"
#include "glm/gtc/constants.hpp" // glm::pi
#include "glm/gtc/type_ptr.hpp"

#include <unistd.h>
#include <vector>
#include <stdlib.h>

//#define DEBUGMODE

#ifdef DEBUGMODE
#define DEBUG(x)	std::cout << x << std::endl
#else
#define DEBUG(x)
#endif

#include "utilities.h"
#include "suspension.h"
#include "graphics.h"







class Graph {
public:
	std::vector<float> values;
	float min, max;
	Graph(float mi, float ma){
		min = mi;
		max = ma;
		for (int x = 0; x < 3000; ++x) {
			values.push_back(0.5f);
		}
	}

	void add(float f){
		std::vector<float>::iterator it = values.begin();
		values.insert(it,f);
		values.pop_back();
	}

	float get(int index){
		return 1.0 / max * values.at(index);
	}
};


float xOffset = 0;
Graph graphEngineVel(0.0f, 1500.0f);
Graph graphClutchVel(0.0f, 1500.0f);
Graph graphClutchAccelVel(0.0f, 800.0f);

bool paused = false;

DistanceTracker shock;



void test(){

	SuspensionLinkage linkage(LINKAGE_14278); 

}






int main(void){


	test();

	


	
	int gra = initGraphics();

	if(gra == -3){
		std::cout << "ERROR IN GRAPHICS!!!" << std::endl;
	}

	dInitODE();
	
	dWorldID mainWorldID = dWorldCreate();
	
	dWorldSetGravity(mainWorldID, 0, -9.81, 0); //-9.81
	
	dSpaceID space = dSimpleSpaceCreate(0);
	
	dGeomID planeGeom = dCreatePlane(space, 0, 1, 0, 0);

	// Wheel 1
	dBodyID wheel1Body = dBodyCreate(mainWorldID);
	dBodySetPosition(wheel1Body, -1, 2, 0);

	dQuaternion wheel1Quaternion;
	dQFromAxisAndAngle(wheel1Quaternion, 0, 0, 1, glm::pi<float>() / 180 * 30.0f);
	dBodySetQuaternion(wheel1Body, wheel1Quaternion);

	dGeomID wheel1Geom = dCreateCylinder(space, 0.5f, 0.5f);
	//dGeomID wheel1Geom = dCreateBox(space, 1, 1, 1);

	dGeomSetBody(wheel1Geom, wheel1Body);
	dMass wheel1Mass;
	
	dMassSetCylinder(&wheel1Mass, 2, 3, 0.5f, 0.5f);
	//dMassSetBox(&wheel1Mass, 2, 1, 1, 1);

	dBodySetMass(wheel1Body, &wheel1Mass);

	dBodyAddForce(wheel1Body, 0, 0, 0);



	// Swingarm
	dBodyID swingBody = dBodyCreate(mainWorldID);
	dBodySetPosition(swingBody, 4, 6, 0);

	dQuaternion swingQuaternion;
	dQFromAxisAndAngle(swingQuaternion, 0, 0, 1, glm::pi<float>() / 180 * 30.0f);
	dBodySetQuaternion(swingBody, swingQuaternion);

	//dGeomID swingGeom = dCreateCylinder(space, 0.5f, 0.5f);
	dGeomID swingGeom = dCreateBox(space, 1, 1, 1);

	dGeomSetBody(swingGeom, swingBody);
	dMass swingMass;
	
	//dMassSetCylinder(&swingMass, 2, 3, 0.5f, 0.5f);
	dMassSetBox(&swingMass, 2, 1, 1, 1);

	dBodySetMass(swingBody, &swingMass);







	dBodyAddForce(swingBody, 0, 0, 0);


	// Contact group
	dJointGroupID contactGroup = dJointGroupCreate(0);


	// Set Other Joints
	dJointID swingJoint = dJointCreateHinge(mainWorldID, 0);
	dJointAttach(swingJoint, swingBody, 0);
	dJointSetHingeAnchor(swingJoint, 4, 6, 0);
	dJointSetHingeAxis(swingJoint, 0,0,1);
	

	dJointID wheelJoint = dJointCreateHinge(mainWorldID, 0);
	dJointAttach(wheelJoint, wheel1Body, swingBody);
	//dJointSetHingeAnchor(wheelJoint, 4, 6, 0);
	dJointSetHingeAxis(wheelJoint, 0,0,1);

	
	const dReal step = 0.0005;


	// Engine
	float controlThrottle = 0.0;
	float controlClutch = 1.0;

	float engineVelocity = 1000;
	float clutchShaftVelocity = 0;

	float engineClutchShaftAverageVelocity = (engineVelocity + clutchShaftVelocity) / 2;
	bool  engineClutchIsEngineFaster;
	float engineClutchDeltaVelocity = engineVelocity - clutchShaftVelocity;
	float engineClutchDeltaTorque;

	float engineMass = 2.7;
	float engineRadius = 0.03;
	float engineInertia = engineMass * engineRadius * engineRadius * 0.5;

	float clutchShaftMass = 10;
	float clutchShaftRadius = 0.1;
	float clutchShaftInertia = clutchShaftMass * clutchShaftRadius * clutchShaftRadius * 0.5;

	float clutchNormalForce  = 190;
	float clutchFriction = 0.4;
	float clutchInnerRadius = 0.065;
	float clutchOuterRadius = 0.08;
	int   clutchContactSurfaces = 16;
	float clutchRadius = (clutchInnerRadius + clutchOuterRadius) / 2.0;
	float clutchSlipTorque;
	bool  clutchLocked;


	DataSet engineTorqueCurveWOTData;
	engineTorqueCurveWOTData.data = std::vector<float> { 0, 1500, 1, 28, 20, 18, 0 };
	Graph1f engineTorqueWOTCurve(engineTorqueCurveWOTData);
	DataSet engineTorqueCurveIdleData;
	engineTorqueCurveIdleData.data = std::vector<float> { 0, 1500, 1, -3, -6, -12, -18 };
	Graph1f engineTorqueIdleCurve(engineTorqueCurveIdleData);


	float engineDriveTorque;
	float clutchDriveTorque;

	float engineTotalTorque;
	float clutchTotalTorque;

	float engineAcceleration = 0;
	float clutchAcceleration = 0;

	DEBUG("EA:   " << engineAcceleration << "   CA:   " << clutchAcceleration);
	DEBUG("EV:   " << engineVelocity << "   CV:   " << clutchShaftVelocity);


	float simTime = 0;
	float scale = 100;

	while(!glfwWindowShouldClose(windowHandle)){
		
		DEBUG(" ---------------------------------------------- Time: " << simTime);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


		clutchSlipTorque = clutchNormalForce * controlClutch * clutchFriction * clutchRadius * clutchContactSurfaces;

		engineDriveTorque = lerp(engineTorqueIdleCurve.evaluate(engineVelocity), engineTorqueWOTCurve.evaluate(engineVelocity), controlThrottle);
		clutchDriveTorque = clutchAcceleration * clutchShaftInertia;

		engineClutchDeltaVelocity = engineVelocity - clutchShaftVelocity;
		engineClutchDeltaTorque = engineDriveTorque - clutchDriveTorque;

		DEBUG("     DRV:    " << engineDriveTorque);
		DEBUG("     CST:    " << clutchSlipTorque);
		DEBUG("     ECT:    " << engineClutchDeltaTorque);

		// Calculate total torques
		if(engineClutchDeltaVelocity > 0) { // Engine is faster
			DEBUG(" Clutch Slipping - Engine +");
			clutchLocked = false;
			engineTotalTorque = engineDriveTorque - clutchSlipTorque;
			clutchTotalTorque = clutchSlipTorque;
			engineClutchIsEngineFaster = true;
		} else if(engineClutchDeltaVelocity < 0) {
			DEBUG(" Clutch Slipping - Engine -");
			clutchLocked = false;
			engineTotalTorque = engineDriveTorque + clutchSlipTorque;
			clutchTotalTorque = -clutchSlipTorque;
			engineClutchIsEngineFaster = false;
		} else { // velocity is same

			if(engineClutchDeltaTorque > clutchSlipTorque) { // Engine is driving more than clutch can hold
				DEBUG(" Clutch Slipping @ Delta 0 - Engine +");
				clutchLocked = false;
				engineTotalTorque = engineDriveTorque - clutchSlipTorque;
				clutchTotalTorque = clutchSlipTorque;
				engineClutchIsEngineFaster = true;
			} else if (engineClutchDeltaTorque < -clutchSlipTorque) { // Engine is slowing down faster than clutch can hold
				DEBUG(" Clutch Slipping @ Delta 0 - Engine -");
				clutchLocked = false;
				engineTotalTorque = engineDriveTorque + clutchSlipTorque;
				clutchTotalTorque = -clutchSlipTorque;
				engineClutchIsEngineFaster = false;
			} else {
				DEBUG(" Clutch Locked");
				clutchLocked = true;
				engineTotalTorque = engineDriveTorque;
				clutchTotalTorque = engineDriveTorque;
			}
		}

		// Calculate delta Velocity (Acceleration)
		if(!clutchLocked) {
			engineAcceleration = engineTotalTorque / engineInertia;
			clutchAcceleration = clutchTotalTorque / clutchShaftInertia;
		} else {
			engineAcceleration = engineTotalTorque / (engineInertia + clutchShaftInertia);
			clutchAcceleration = clutchTotalTorque / (engineInertia + clutchShaftInertia);
		}

		// Limit speed
		if(engineVelocity > 1300){
			engineVelocity = 1300;
			engineAcceleration = 0;
			if(clutchAcceleration > 0){
				//clutchAcceleration = 0;
			}
		}

		// Apply Acceleration to Elements
		engineVelocity += engineAcceleration * step;
		clutchShaftVelocity += clutchAcceleration * step;


		// Check for velocity join
		engineClutchDeltaVelocity = engineVelocity - clutchShaftVelocity;

		if((engineClutchDeltaVelocity > 0 && !engineClutchIsEngineFaster) || (engineClutchDeltaVelocity < 0 && engineClutchIsEngineFaster)) {
			// Engine Delta Velocity changed polarity
			DEBUG(" LOCKING ");
			engineClutchShaftAverageVelocity =  (engineVelocity + clutchShaftVelocity) / 2;
			engineVelocity = engineClutchShaftAverageVelocity;
			clutchShaftVelocity = engineClutchShaftAverageVelocity;
		}


		DEBUG("EA:   " << engineAcceleration << "   CA:   " << clutchAcceleration);
		DEBUG("EV:   " << engineVelocity << "   CV:   " << clutchShaftVelocity);

		graphEngineVel.add(engineVelocity);
		graphClutchVel.add(clutchShaftVelocity);
		graphClutchAccelVel.add(clutchAcceleration);

		// ----------------------------------------------------------------------------------------

		if(glfwGetKey(windowHandle, GLFW_KEY_P) == GLFW_PRESS){
			if(controlThrottle < 1.0){
				controlThrottle += 0.02f;
			}
			if(controlThrottle > 1){
				controlThrottle = 1.0f;
						}
		}
		if(glfwGetKey(windowHandle, GLFW_KEY_O) == GLFW_PRESS){
			if(controlThrottle > 0.0){
				controlThrottle -= 0.02f;
			}
			if(controlThrottle < 0){
				controlThrottle = 0.0f;
						}
		}

		if(glfwGetKey(windowHandle, GLFW_KEY_R) == GLFW_PRESS){
			if(controlClutch < 1.0){
				controlClutch += 0.02f;
			}
			if(controlClutch > 1){
				controlClutch = 1.0f;
			}
		}
		if(glfwGetKey(windowHandle, GLFW_KEY_E) == GLFW_PRESS){
			if(controlClutch > 0.0){
				controlClutch -= 0.02f;
			}
			if(controlClutch < 0){
				controlClutch = 0.0f;
			}
		}

		if(glfwGetKey(windowHandle, GLFW_KEY_SPACE) == GLFW_PRESS){
			engineVelocity = 0;
			clutchShaftVelocity = 0;
		}
		if(glfwGetKey(windowHandle, GLFW_KEY_V) == GLFW_PRESS){
			clutchShaftVelocity += 3;
		}
		if(glfwGetKey(windowHandle, GLFW_KEY_C) == GLFW_PRESS){
			clutchShaftVelocity -= 3;
		}

		DEBUG(" THROTTLE: " << controlThrottle);
		DEBUG(" CLUTCH:   " << controlClutch);

		
		if(glfwGetKey(windowHandle, GLFW_KEY_DOWN) == GLFW_PRESS){
			if(cameraPositionZ < 500.0){
				cameraPositionZ += 0.2f;
			}
			if(cameraPositionZ > 500){
				cameraPositionZ = 500.0f;
			}
		}

		if(glfwGetKey(windowHandle, GLFW_KEY_UP) == GLFW_PRESS){
			if(cameraPositionZ > -500.0){
				cameraPositionZ -= 0.2f;
			}
			if(cameraPositionZ < -500){
				cameraPositionZ = -500.0f;
			}
		}

		if(glfwGetKey(windowHandle, GLFW_KEY_RIGHT) == GLFW_PRESS){
			if(cameraPositionX < 500.0){
				cameraPositionX += 0.2f;
			}
			if(cameraPositionX > 500){
				cameraPositionX = 500.0f;
			}
		}

		if(glfwGetKey(windowHandle, GLFW_KEY_LEFT) == GLFW_PRESS){
			if(cameraPositionX > -500.0){
				cameraPositionX -= 0.2f;
			}
			if(cameraPositionX < -500){
				cameraPositionX = -500.0f;
			}
		}

		if(glfwGetKey(windowHandle, GLFW_KEY_N) == GLFW_PRESS){
			cameraRotZ -= 0.02f;	
		}
		if(glfwGetKey(windowHandle, GLFW_KEY_M) == GLFW_PRESS){
			cameraRotZ += 0.02f;	
		}

		if(glfwGetKey(windowHandle, GLFW_KEY_P) == GLFW_PRESS){
			dBodyAddForce(wheel1Body, 80, 0, 0);	
		}
		if(glfwGetKey(windowHandle, GLFW_KEY_O) == GLFW_PRESS){
			dBodyAddForce(wheel1Body, -40, 0, 0);	
		}

		if(glfwGetKey(windowHandle, GLFW_KEY_SPACE) == GLFW_PRESS){
			dBodyAddForce(wheel1Body, 0, 30, 0);	
		}

		if(glfwGetKey(windowHandle, GLFW_KEY_K) == GLFW_PRESS){
			//dBodyAddForce(wheel1Body, 10, 0, 0);	
			dBodyAddTorque(wheel1Body, 0, 0, 10);
		}
		if(glfwGetKey(windowHandle, GLFW_KEY_L) == GLFW_PRESS){
			//dBodyAddForce(wheel1Body, 10, 0, 0);	
			dBodyAddTorque(wheel1Body, 0, 0, -10);
		}

		if(glfwGetKey(windowHandle, GLFW_KEY_J) == GLFW_PRESS){
			//dBodyAddForce(wheel1Body, 10, 0, 0);	
			dBodyAddTorque(wheel1Body, 0, 50, 0);
		}

		if(glfwGetKey(windowHandle, GLFW_KEY_T) == GLFW_PRESS){
			dJointSetHingeParam(swingJoint, dParamHiStop, 0.2f);
			dJointSetHingeParam(swingJoint, dParamLoStop, -0.0f);
		}

		
		
		
		
		// Physics
		dJointGroupEmpty(contactGroup);
		
		dContactGeom contactsOfBoxes[10];

		// Wheel 1 collision
		int numOfContactsFound = dCollide(wheel1Geom, planeGeom, 6, &contactsOfBoxes[0], sizeof(dContactGeom));

		std::vector<ContactInfo> contactInfos;

		std::vector<dJointFeedback> feedbacks;

		if(numOfContactsFound > 0){

			dSurfaceParameters surface;
			surface.mode = dContactBounce | dContactApprox1 | dContactRolling;
			surface.mu = 1;
			surface.mu2 = 0;
			surface.rho = 0;
			surface.rho2 = 0;
			surface.bounce = 0;
			surface.bounce_vel = 0;
			surface.soft_erp = 0;
			surface.soft_cfm = 0;
			surface.motion1 = 0;
			surface.motion2 = 0;
			surface.motionN = 0;
			surface.slip1 = 0;
			surface.slip2 = 0;

			for(int x = 0; x < numOfContactsFound; x++){
				dContact contactData;
				contactData.surface = surface;
				contactData.geom = contactsOfBoxes[x];

				dJointFeedback newFeedback;

				dJointID contactJoint = dJointCreateContact(mainWorldID, contactGroup, &contactData);
				dJointSetFeedback(contactJoint, &newFeedback);
				
				dJointAttach(contactJoint, wheel1Body, 0);

				

				//dxJoint::Info2* jointInfo;

				ContactInfo info;
				info.contact = contactData;
				info.feedback = newFeedback;

				//std::cout << "RAW " << newFeedback.f1[0] << std::endl;
				feedbacks.push_back(newFeedback);

				contactInfos.push_back(info);
			}
		}

		glUseProgram(shaderprogram);

		shock.previousDistance = shock.distance;
		// Step world
		dWorldStep(mainWorldID, step);


		std::vector<dJointFeedback>::iterator feedIt;
		for(feedIt = feedbacks.begin(); feedIt != feedbacks.end(); feedIt++){
			//std::cout << "Zt " << (*feedIt).t1[2] << std::endl;
		}

		float finalTorque[4] = { 0, 0, 0, 1 };
		const float* torquePosRaw = dBodyGetPosition(wheel1Body);
		float torquePosition[4] = { torquePosRaw[0], torquePosRaw[1], torquePosRaw[2], 1 };

		std::vector<ContactInfo>::iterator infoIt;
		for(infoIt = contactInfos.begin(); infoIt != contactInfos.end(); infoIt++){
			finalTorque[0] += (*infoIt).feedback.t1[0];
			finalTorque[1] += (*infoIt).feedback.t1[1];
			finalTorque[2] += (*infoIt).feedback.t1[2];
			//std::cout << "X " << (*infoIt).feedback.f1[0] << std::endl;
			//render_arrow((*infoIt).feedback.f1, (*infoIt).contact.geom.pos);
			
			//render_arrow((*infoIt).contact.geom.normal, (*infoIt).contact.geom.pos);
		}

		float length = glm::length(glm::vec3(finalTorque[0],finalTorque[1],finalTorque[2]));
		//std::cout << "Torque Strength: " << length << std::endl;

		float forwardStrength = glm::dot(glm::vec3(finalTorque[0],finalTorque[1],finalTorque[2]),
		glm::vec3(0,0,1));
		//std::cout << "Torque Forward: " << forwardStrength << std::endl;

		float forwardVectorTorque[4] = { 0, 0, forwardStrength, 1 };

		//render_arrow(&finalTorque[0], &torquePosition[0]);
		render_arrow(&forwardVectorTorque[0], &torquePosition[0]);

		//std::cout << "X " << tot << std::endl;


		// DistanceTracker
		
		

		// -------------------------- Render ---------------------------------------------------------------

		

		// ------------------------------------------- Wheel 1

		const float* wheel1BodyPos = dBodyGetPosition(wheel1Body);
		const float* wheel1Quat = dBodyGetQuaternion(wheel1Body);

		render_nullbox(DRAW_METHOD_WIRES, wheel1BodyPos[0], wheel1BodyPos[1], wheel1BodyPos[2], 1, wheel1Quat);


		// ------------------------------------------- Swing

		const float* swingBodyPos = dBodyGetPosition(swingBody);
		const float* swingQuat = dBodyGetQuaternion(swingBody);

		render_nullbox(DRAW_METHOD_WIRES, swingBodyPos[0], swingBodyPos[1], swingBodyPos[2], 1, swingQuat);


		// ------------------------------------------- Plane
		render_plane();
		


		// Debug
		shock.distance = glm::distance(glm::vec3(wheel1BodyPos[0], wheel1BodyPos[1], wheel1BodyPos[2]), 
										glm::vec3(0, 0, 0));
		//std::cout << "Distance: " << shock.distance << "\t\tDelta: " << (shock.distance - shock.previousDistance) << std::endl;


		glfwSwapBuffers(windowHandle);
		
		glfwPollEvents();
		usleep(1000000 / 250);
		simTime += step;
	}
	
	/* Cleanup all the things we bound and allocated */
	glUseProgram(0);
	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glDetachShader(shaderprogram, vertexshader);
	glDetachShader(shaderprogram, fragmentshader);
	glDeleteProgram(shaderprogram);
	glDeleteShader(vertexshader);
	glDeleteShader(fragmentshader);
	glDeleteBuffers(2, vbo);
	glDeleteVertexArrays(1, &vao);
	free(vertexsource);
	free(fragmentsource);


	dCloseODE();
	
	glfwTerminate();
	return 0;
}

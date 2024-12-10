/****************************************************
 * 2016-2023 Eric Bachmann and Mike Zmuda
 * All Rights Reserved.
 * NOTICE:
 * Dissemination of this information or reproduction
 * of this material is prohibited unless prior written
 * permission is granted.
 ****************************************************/
#include "raytracer.h"
#include "ishape.h"
#include "io.h"

 /**
  * @fn	RayTracer::RayTracer(const color &defa)
  * @brief	Constructs a raytracers.
  * @param	defa	The clear color.
  */

RayTracer::RayTracer(const color& defa)
	: defaultColor(defa) {
}

/**
 * @fn	void RayTracer::raytraceScene(FrameBuffer &frameBuffer, int depth, const IScene &theScene) const
 * @brief	Raytrace scene
 * @param [in,out]	frameBuffer	Framebuffer.
 * @param 		  	depth	   	The current depth of recursion.
 * @param 		  	theScene   	The scene.
 */

void RayTracer::raytraceScene(FrameBuffer& frameBuffer, int depth,
	const IScene& theScene) const {
	const RaytracingCamera& camera = *theScene.camera;
	const vector<VisibleIShapePtr>& oObjs = theScene.opaqueObjs;
	const vector<TransparentIShapePtr>& tObjs = theScene.transparentObjs;
	const vector<LightSourcePtr>& lights = theScene.lights;
	color defaultColor = frameBuffer.getClearColor();

	for (int y = 0; y < frameBuffer.getWindowHeight(); ++y) {
		for (int x = 0; x < frameBuffer.getWindowWidth(); ++x) {
			DEBUG_PIXEL = (x == xDebug && y == yDebug);
			if (DEBUG_PIXEL) {
				cout << "";
			}

			Ray ray = camera.getRay(x, y);
			OpaqueHitRecord oHit;
			TransparentHitRecord tHit;
			VisibleIShape::findIntersection(ray, oObjs, oHit);
			TransparentIShape::findIntersection(ray, tObjs, tHit);

			color finalCol = black;
			color defaultColor = paleGreen;
			if (oHit.t != FLT_MAX && tHit.t == FLT_MAX) { // Hit only opaque, not transparent
				finalCol = calculateIlluminateColor(ray, oHit, theScene, finalCol, oObjs);
				if (depth > 0) { // Reflections
					color reflectionColor = traceIndividualRay(ray, theScene, depth);
					finalCol += reflectionColor;
				}
				frameBuffer.setColor(x, y, finalCol);
			}
			else if (oHit.t == FLT_MAX && tHit.t != FLT_MAX) { // Hit transparent, Not opaque
				// Check for back face
				bool isBackFace = glm::dot(ray.dir, oHit.normal) > 0;
				tHit.normal = isBackFace ? -tHit.normal : tHit.normal;

				finalCol = (1 - tHit.alpha) * defaultColor + tHit.alpha * tHit.transColor;
				frameBuffer.setColor(x, y, finalCol);
			}
			else if (oHit.t != FLT_MAX && tHit.t != FLT_MAX) { // Hit both types
				if (oHit.t <= tHit.t) { // Opaque is closer than transparent
					finalCol = calculateIlluminateColor(ray, oHit, theScene, finalCol, oObjs);
				}
				else { // Transparent is closer than opaque
					color oColor = black;
					oColor = calculateIlluminateColor(ray, oHit, theScene, finalCol, oObjs);
					finalCol = (1 - tHit.alpha) * oColor + tHit.alpha * tHit.transColor;
				}
				if (depth > 0) { // Reflections
					color reflectionColor = traceIndividualRay(ray, theScene, depth);
					finalCol += reflectionColor;
				}
				frameBuffer.setColor(x, y, finalCol);
			}
			else { // Hit nothing
				frameBuffer.setColor(x, y, defaultColor);
			}
			frameBuffer.showAxes(x, y, ray, 0.25);			// Displays R/x, G/y, B/z axes
		}
	}

	frameBuffer.showColorBuffer();
}

color RayTracer::calculateIlluminateColor(Ray& ray, OpaqueHitRecord& oHit, const IScene& theScene,
		color col, const vector<VisibleIShapePtr>& oObjs) const {
	// Check for back face
	bool isBackFace = glm::dot(ray.dir, oHit.normal) > 0;
	oHit.normal = isBackFace ? -oHit.normal : oHit.normal;

	// Color for Each Light in scene
	for (unsigned int i = 0; i < theScene.lights.size(); i++) {
		LightSourcePtr light = theScene.lights[i];
		bool inShadow = light->pointIsInAShadow(oHit.interceptPt, oHit.normal, oObjs, theScene.camera->getFrame());
		col += light->illuminate(oHit.interceptPt, oHit.normal, oHit.material, theScene.camera->getFrame(), inShadow);
	}

	// Apply texture color blending
	if (oHit.texture != nullptr) {
		color texCol = oHit.texture->getPixelUV(oHit.u, oHit.v);
		col = 0.5 * col + 0.5 * texCol;
	}

	return col;
}

/**
 * @fn	color RayTracer::traceIndividualRay(const Ray &ray,
 *											const IScene &theScene,
 *											int recursionLevel) const
 * @brief	Trace an individual ray.
 * @param	ray			  	The ray.
 * @param	theScene	  	The scene.
 * @param	recursionLevel	The recursion level.
 * @return	The color to be displayed as a result of this ray.
 */

color RayTracer::traceIndividualRay(const Ray& ray, const IScene& theScene, int recursionLevel) const {
	// Check if max recursion
	if (recursionLevel <= 0) {
		return black;
	}
	// Get hit point and normal
	OpaqueHitRecord oHit;
	const vector<VisibleIShapePtr>& oObjs = theScene.opaqueObjs;
	VisibleIShape::findIntersection(ray, oObjs, oHit);

	// If hit something, make new reflection ray
	if (oHit.t != FLT_MAX) {
		Ray reflectionRay = Ray(oHit.interceptPt + oHit.normal * EPSILON, ray.dir - 2 * (glm::dot(ray.dir, oHit.normal)) * oHit.normal);
		// Check to see if this ray hits
		OpaqueHitRecord rHit;
		VisibleIShape::findIntersection(reflectionRay, oObjs, rHit);
		if (rHit.t != FLT_MAX) {
			// if yes, get color and call function again
			return (calculateIlluminateColor(reflectionRay, rHit, theScene, black, oObjs) + traceIndividualRay(reflectionRay, theScene, recursionLevel - 1)) * .3;
		}	
	}
	return black;
}

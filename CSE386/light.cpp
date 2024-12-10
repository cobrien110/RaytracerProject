/****************************************************
 * 2016-2023 Eric Bachmann and Mike Zmuda
 * All Rights Reserved.
 * NOTICE:
 * Dissemination of this information or reproduction
 * of this material is prohibited unless prior written
 * permission is granted.
 ****************************************************/

#include "light.h"
#include "io.h"
#include "ishape.h"

 /**
  * @fn	color ambientColor(const color &matAmbient, const color &lightColor)
  * @brief	Computes the ambient color produced by a single light at a single point.
  * @param	mat  	Ambient material property.
  * @param	lightColor	Light's color.
  * @return	Ambient color.
   */

color ambientColor(const color& mat, const color& lightColor) {
	/* CSE 386 - todo  */
	return glm::clamp(mat * lightColor, 0.0, 1.0);
}

/**
 * @fn	color diffuseColor(const color &mat, const color &light, const dvec3 &l, const dvec3 &n)
 * @brief	Computes diffuse color produce by a single light at a single point.
 * @param	mat		 	Material.
 * @param	lightColor	 	The light's color.
 * @param	l		 	Light vector.
 * @param	n		 	Normal vector.
 * @return	Diffuse color.
 */

color diffuseColor(const color& mat, const color& lightColor,
	const dvec3& l, const dvec3& n) {
	/* CSE 386 - todo  */
	double dp = glm::dot(l, n);
	return mat * lightColor * glm::max(0.0, dp);
}

/**
 * @fn	color specularColor(const color &mat, const color &lightColor, double shininess,
 *							const dvec3 &r, const dvec3 &v)
 * @brief	Computes specular color produce by a single light at a single point.
 * @param	mat		 	Material.
 * @param	lightColor	 	The light's color.
 * @param	shininess	Material shininess.
 * @param	r		 	Reflection vector.
 * @param	v		 	Viewing vector.
 * @return	Specular color.
 */

color specularColor(const color& mat, const color& lightColor,
	double shininess,
	const dvec3& r, const dvec3& v) {
	/* CSE 386 - todo  */
	double dp = glm::clamp(glm::dot(r, v), 0.0, 1.0);
	double dpP = glm::pow(dp, shininess);
	return mat * lightColor * dpP;
}

/**
 * @fn	color totalColor(const Material &mat, const color &lightColor,
 *						const dvec3 &viewingDir, const dvec3 &normal,
 *						const dvec3 &lightPos, const dvec3 &intersectionPt,
 *						bool attenuationOn, const LightAttenuationParameters &ATparams)
 * @brief	Color produced by a single light at a single point. This includes the summation
 * of the ambient, diffuse, and specular components. Attenuation is also applied, if applicable.
 * @param	mat			  	Material.
 * @param	lightColor	  	The light's color.
 * @param	v	  			The v vector.
 * @param	n   		  	Normal vector.
 * @param	lightPos	  	Light position.
 * @param	intersectionPt	(x,y,z) of intersection point.
 * @param	attenuationOn 	true if attenuation is on.
 * @param	ATparams	  	Attenuation parameters.
 * @return	Color produced by a single light at a single point. This includes the summation
 * of the ambient, diffuse, and specular components. Attenuation is also applied, if applicable.
 */

color totalColor(const Material& mat, const color& lightColor,
	const dvec3& v, const dvec3& n,
	const dvec3& lightPos, const dvec3& intersectionPt,
	bool attenuationOn,
	const LightATParams& ATparams) {
	/* CSE 386 - todo  */
	dvec3 l = glm::normalize(lightPos - intersectionPt);
	dvec3 r = (2 * glm::dot(l, n) * n - l);
	double d = glm::distance(lightPos, intersectionPt);

	double at;
	if (attenuationOn) {
		at = ATparams.factor(d);
	}
	else {
		at = 1;
	}

	color amb = ambientColor(mat.ambient, lightColor);
	color dif = diffuseColor(mat.diffuse, lightColor, l, n);
	color spec = specularColor(mat.specular, lightColor, mat.shininess, r, v);
	return glm::clamp(amb + at * (dif + spec), 0.0, 1.0);
}

/**
 * @fn	color PositionalLight::illuminate(const dvec3 &interceptWorldCoords,
 *										const dvec3 &normal, const Material &material,
 *										const Frame &eyeFrame, bool inShadow) const
 * @brief	Computes the color this light produces in RAYTRACING applications.
 * @param	interceptWorldCoords	(x, y, z) at the intercept point.
 * @param	normal				The normal vector.
 * @param	material			The object's material properties.
 * @param	eyeFrame			The coordinate frame of the camera.
 * @param	inShadow			true if the point is in a shadow.
 * @return	The color produced at the intercept point, given this light.
 */

color PositionalLight::illuminate(const dvec3& interceptWorldCoords,
	const dvec3& normal,
	const Material& material,
	const Frame& eyeFrame,
	bool inShadow) const {
	/* CSE 386 - todo  */
	if (!isOn) {
		return black;
	}
	else {
		/// call totalColor
		dvec3 v = pointingVector(interceptWorldCoords, eyeFrame.origin);
		if (inShadow) {
			return ambientColor(material.ambient, lightColor);
		}
		else {
			return totalColor(material, lightColor, v, normal, pos, interceptWorldCoords, attenuationIsTurnedOn, atParams);
		}
	}
	//return material.diffuse;
}

/*
* @fn PositionalLight::actualPosition(const Frame& eyeFrame) const
* @brief	Returns the global world coordinates of this light.
* @param	eyeFrame	The camera's frame
* @return	The global world coordinates of this light. This will be the light's
*			raw position. Or, it will be the position relative to the camera's
*			frame (transformed from camera's frame into the world coordinate frame).
*/

dvec3 PositionalLight::actualPosition(const Frame& eyeFrame) const {
	return isTiedToWorld ? pos : eyeFrame.frameCoordsToGlobalCoords(pos);
}

/**
* @fn	bool PositionalLight::pointIsInAShadow(const dvec3& intercept, const dvec3& normal, const vector<VisibleIShapePtr>& objects, const Frame& eyeFrame) const
* @brief	Determines if an intercept point falls in a shadow.
* @param	intercept	the position of the intercept.
* @param	normal		the normal vector at the intercept point
* @param	objects		the collection of opaque objects in the scene
*/

bool PositionalLight::pointIsInAShadow(const dvec3& intercept,
	const dvec3& normal,
	const vector<VisibleIShapePtr>& objects,
	const Frame& eyeFrame) const {
	if (DEBUG_PIXEL) {
		cout << "";
	}
	Ray shadowFeeler = getShadowFeeler(intercept, normal, eyeFrame);
	OpaqueHitRecord hit;
	VisibleIShape::findIntersection(shadowFeeler, objects, hit);
	return hit.t != FLT_MAX && hit.t < glm::distance(intercept, pos);
}

/**
* @fn	Ray PositionalLight::getShadowFeeler(const dvec3& interceptWorldCoords, const dvec3& normal, const Frame &eyeFrame) const
* @brief	Returns the shadow feeler for this light.
* @param	interceptWorldCoords	the position of the intercept.
* @param	normal		The normal vector at the intercept point
* @param	eyeFrame	The coordinate frame of the camera.
*/

Ray PositionalLight::getShadowFeeler(const dvec3& interceptWorldCoords,
	const dvec3& normal,
	const Frame& eyeFrame) const {
	/* 386 - todo */
	dvec3 origin = interceptWorldCoords + EPSILON * normal;
	dvec3 dir = pointingVector(interceptWorldCoords, pos);
	Ray shadowFeeler(origin, dir);
	return shadowFeeler;
}

/**
 * @fn	color SpotLight::illuminate(const dvec3 &interceptWorldCoords,
 *									const dvec3 &normal, const Material &material,
 *									const Frame &eyeFrame, bool inShadow) const
 * @brief	Computes the color this light produces in raytracing applications.
 * @param	interceptWorldCoords				The surface properties of the intercept point.
 * @param	normal					The normal vector.
 * @param	material			The object's material properties.
 * @param	eyeFrame			The coordinate frame of the camera.
 * @param	inShadow			true if the point is in a shadow.
 * @return	The color produced at the intercept point, given this light.
 */

color SpotLight::illuminate(const dvec3& interceptWorldCoords,
	const dvec3& normal,
	const Material& material,
	const Frame& eyeFrame,
	bool inShadow) const {
	if (isInSpotlightCone(pos, spotDir, fov, interceptWorldCoords)) {
		return PositionalLight::illuminate(interceptWorldCoords, normal, material, eyeFrame, inShadow);
	}
	else {
		return black;
	}
}

/**
* @fn	void setDir (double dx, double dy, double dz)
* @brief	Sets the direction of the spotlight.
* @param	dx		x component of the direction
* @param	dy		y component of the direction
* @param	dz		z component of the direction
*/

void SpotLight::setDir(double dx, double dy, double dz) {
	spotDir = glm::normalize(dvec3(dx, dy, dz));
}

/**
* @fn	SpotLight::isInSpotlightCone(const dvec3& spotPos, const dvec3& spotDir, double spotFOV, const dvec3& intercept)
* @brief	Determines if an intercept point falls within a spotlight's cone.
* @param	spotPos		where the spotlight is positioned
* @param	spotDir		normalized direction of spotlight's pointing direction
* @param	spotFOV		spotlight's field of view, which is 2X of the angle from the viewing axis
* @param	intercept	the position of the intercept.
*/

bool SpotLight::isInSpotlightCone(const dvec3& spotPos,
	const dvec3& spotDir,
	double spotFOV,
	const dvec3& intercept) {
	dvec3 l = pointingVector(intercept, spotPos);
	double spotCos = glm::dot(-l, spotDir);
	double cutoffAngle = spotFOV / 2.0;
	return spotCos > glm::cos(cutoffAngle);
}

#include "VirtualTrackball.h"
#include <cmath>
#include <iostream>

glm::mat4 quatToMat4(glm::quat m_q) 
{
	glm::mat4 returnMatrix;
	const float s = 2.0f;

	//Normalizing if the quaternion is not of length 1
	if(abs(glm::length(m_q) - 1.0f) > 0.000001f)
		m_q = glm::normalize(m_q);

	returnMatrix = glm::mat4(
		1 - (s * (m_q.y*m_q.y + m_q.z*m_q.z)), 
		s * (m_q.x * m_q.y - m_q.w * m_q.z),   
		s * (m_q.x * m_q.z + m_q.w * m_q.y),   
		0,
		s * (m_q.x * m_q.y + m_q.w * m_q.z),    
		1 - (s * (m_q.x*m_q.x + m_q.z*m_q.z)), 
		s * (m_q.y * m_q.z - m_q.w * m_q.x),   
		0,
		s * (m_q.x * m_q.z - m_q.w * m_q.y),    
		s * (m_q.y * m_q.z + m_q.w * m_q.x),   
		1 - (s * (m_q.x*m_q.x + m_q.y*m_q.y)), 
		0, 0, 0, 0, 1);
	return returnMatrix;
}

VirtualTrackball::VirtualTrackball() 
{
	quat_old.w = 1.0;
	quat_old.x = 0.0;
	quat_old.y = 0.0;
	quat_old.z = 0.0;
	rotating = false;
}

VirtualTrackball::~VirtualTrackball() 
{

}

void VirtualTrackball::rotateBegin(int x, int y) 
{
	rotating = true;
	point_on_sphere_begin = getClosestPointOnUnitSphere(x, y);
}

void VirtualTrackball::rotateEnd(int x, int y) 
{
	rotating = false;
	quat_old = quat_new;
}

glm::mat4 VirtualTrackball::rotate(int x, int y, float rotSpeed) 
{
	//If not rotating, simply return the old rotation matrix
	if (!rotating) return quatToMat4(quat_old);

	glm::vec3 point_on_sphere_end; //Current point on unit sphere
	glm::vec3 axis_of_rotation; //axis of rotation
	float theta = 0.0f; //angle of rotation

	point_on_sphere_end = getClosestPointOnUnitSphere(x, y);

	/**
	  * Find axis of rotation and angle here. Construct the
	  * rotation quaternion using glm helper functions
	  */

	point_on_sphere_begin = glm::normalize(point_on_sphere_begin);
	point_on_sphere_end = glm::normalize(point_on_sphere_end);

	//The axis of rotation that we rotate arround is found by the cross product between
	//the end and begin point on the sphere.
	axis_of_rotation = glm::cross(point_on_sphere_end, point_on_sphere_begin);
	
	//Finding the amount of degrees to rotate by taking dot product between the two points we have on the sphere.
	//Muptiplying with a rotationSpeed constant to fit the speed of the rotation better to my liking.
	theta = glm::degrees(glm::acos(glm::dot(point_on_sphere_begin, point_on_sphere_end)))*rotSpeed;

	//Constructing the new quaternion using the old quat, and our newly found degrees and axis of rotation.
	quat_new = glm::rotate(quat_old, theta, axis_of_rotation);

	return quatToMat4(quat_new);
}

void VirtualTrackball::setWindowSize(int w, int h) 
{
	this->w = w;
	this->h = h;
}

glm::vec2 VirtualTrackball::getNormalizedWindowCoordinates(int x, int y) 
{
	glm::vec2 coord = glm::vec2(x / static_cast<float>(w) - 0.5f, 
		0.5f - static_cast<float>(y) / h);

	return coord;
}

glm::vec3 VirtualTrackball::getClosestPointOnUnitSphere(int x, int y) 
{
	glm::vec2 normalized_coords;
	glm::vec3 point_on_sphere;
	float k;//< The distance from the center of the sphere, to the point of click-intersection.
	float r = 0.5f;
	
	normalized_coords = getNormalizedWindowCoordinates(x, y);
	
	k = sqrt(static_cast<float>((normalized_coords.x * normalized_coords.x) 
							  + (normalized_coords.y * normalized_coords.y)));

	//Mouse is on the hyperbolic sheet
	if (k > r*r/2)
		point_on_sphere = glm::vec3(normalized_coords.x * 2.0f, 
									normalized_coords.y * 2.0f, 
									((r*r)/2)/k);

	//Mouse is inside the sphere
	else
		point_on_sphere = glm::vec3(normalized_coords.x * 2.0f, 
									normalized_coords.y * 2.0f, 
									sqrt(1 - 4 * (k*k)));


	return point_on_sphere;
}
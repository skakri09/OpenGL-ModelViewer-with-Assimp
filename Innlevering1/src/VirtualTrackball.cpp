#include "VirtualTrackball.h"
#include <cmath>
#include <iostream>

glm::mat4 quatToMat4(glm::quat m_q) 
{
	//glm::mat4 retMat;

	//if(abs(glm::length(m_q) - 1.0f) > 0.000001f)
	//{
	//	m_q = glm::normalize(m_q);
	//}

	//retMat[0][0] = (1 - (2*((pow(m_q.y,2)) + (pow(m_q.z,2)))));
	//retMat[0][1] = 2*( (m_q.x * m_q.y) - (m_q.w * m_q.z));
	//retMat[0][2] = 2*( (m_q.x * m_q.z) + (m_q.w*m_q.y));
	//retMat[0][3] = 0;
	//retMat[1][0] = 2*((m_q.x*m_q.y) + (m_q.w*m_q.z));
	//retMat[1][1] = 1 - (2*(pow(m_q.x,2) + pow(m_q.x,2)));
	//retMat[1][2] = 2*((m_q.y*m_q.z) - (m_q.w*m_q.x));
	//retMat[1][3] = 0;
	//retMat[2][0] = 2*((m_q.x*m_q.z) - (m_q.w*m_q.y));
	//retMat[2][1] = 2*((m_q.y*m_q.z) + (m_q.w*m_q.x));
	//retMat[2][2] = 1-(2*(pow(m_q.x,2)) + (pow(m_q.y,2)));
	//retMat[2][3] = 0;
	//retMat[3][0] = 0;
	//retMat[3][1] = 0;
	//retMat[3][2] = 0;
	//retMat[3][3] = 1;

	//return retMat;
	const float s = 2.0f;
	glm::mat4 retMat;

	if(abs(glm::length(m_q) - 1.0f) > 0.000001f)
		m_q = glm::normalize(m_q);

	retMat = glm::mat4(
		1 - (s * (pow(m_q.y, 2) + pow(m_q.z, 2))), 
		s * (m_q.x * m_q.y - m_q.w * m_q.z),   
		s * (m_q.x * m_q.z + m_q.w * m_q.y),   
		0,
		s * (m_q.x * m_q.y + m_q.w * m_q.z),    
		1 - (s * (pow(m_q.x, 2) + pow(m_q.z, 2))), 
		s * (m_q.y * m_q.z - m_q.w * m_q.x),   
		0,
		s * (m_q.x * m_q.z - m_q.w * m_q.y),    
		s * (m_q.y * m_q.z + m_q.w * m_q.x),   
		1 - (s * (pow(m_q.x, 2) + pow(m_q.y, 2))), 
		0, 0, 0, 0, 1);
	return retMat;
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

glm::mat4 VirtualTrackball::rotate(int x, int y) 
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
	
	axis_of_rotation = glm::cross(point_on_sphere_begin, point_on_sphere_end);
	
	point_on_sphere_begin = glm::normalize(point_on_sphere_begin);
	point_on_sphere_end = glm::normalize(point_on_sphere_end);


	theta = glm::degrees(glm::acos(glm::dot(point_on_sphere_begin, point_on_sphere_end)));
	//theta*= 50;//make the multiplier an adjustable variable
	quat_new = glm::rotate(quat_old, -theta, axis_of_rotation);
	//std::cout << "Angle: " << theta << std::endl;
	//std::cout << "Axis: " << axis_of_rotation.x << " " << axis_of_rotation.y << " " << axis_of_rotation.z << std::endl;

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

	//std::cout << "Normalized coordinates: " << coord.x << ", " << coord.y << std::endl;

	return coord;
}

glm::vec3 VirtualTrackball::getClosestPointOnUnitSphere(int x, int y) 
{
	glm::vec2 normalized_coords;
	glm::vec3 point_on_sphere;
	float k;

	normalized_coords = getNormalizedWindowCoordinates(x, y);
	
	/**
	  * Find the point on the unit sphere here from the
	  * normalized window coordinates
	  */
	k = sqrt( static_cast<float>(normalized_coords.x * normalized_coords.x) + static_cast<float>(normalized_coords.y * normalized_coords.y));

	if(k >= 0.5f)
	{
		point_on_sphere = glm::vec3(normalized_coords.x/k, normalized_coords.y/k, 0.0f);
		//std::cout << "outside," << k << std::endl;
	}
	else
	{
		point_on_sphere = glm::vec3(2.0f*normalized_coords.x, 2.0f*normalized_coords.y, sqrt(1.0f - ( 4.0f *pow(k, 2.0f))));
		//std::cout << "inside, " << k <<  std::endl;
	}
	
	//std::cout << "Point on sphere: " << point_on_sphere.x << ", " << point_on_sphere.y << ", " << point_on_sphere.z << std::endl;

	return point_on_sphere;
}
/* std.vert - basic transformation, no lighting */

//// Application provided tangent space vectors.
//// These vectors are normalized.
//attribute vec3 attrTangent;
//attribute vec3 attrBitangent;

// shader output
varying mat3 tangentToEyeMatrix;
varying vec3 position; // vertex position in eye space.
varying vec3 normal;   // vertex normal in eye space.

//
// entry point
//
void main( void )
{
	// pass through attributes.
	gl_Position = ftransform();
	gl_TexCoord[0] = gl_MultiTexCoord0;
	gl_FrontColor = gl_Color;

	// pass vertex attributes to the fragment shader.
	normal = gl_NormalMatrix * gl_Normal;
	position = ( gl_ModelViewMatrix * gl_Vertex ).xyz;

	// Tangent Space Computing
	// http://www.ozone3d.net/tutorials/mesh_deformer_p2.php#tangent_space
	// http://www.ozone3d.net/tutorials/bump_mapping_p3.php
	vec3 tangent; 
	vec3 binormal; 
	vec3 c1 = cross(gl_Normal, vec3(0.0, 0.0, 1.0)); 
	vec3 c2 = cross(gl_Normal, vec3(0.0, 1.0, 0.0)); 
	if(length(c1)>length(c2))
	{
		tangent = c1;	
	}
	else
	{
		tangent = c2;	
	}
	tangent = normalize(tangent);
	binormal = normalize(cross(gl_Normal, tangent));

	// setup tangent-to-eye matrix.
	tangentToEyeMatrix[0] = gl_NormalMatrix * tangent;
	tangentToEyeMatrix[1] = gl_NormalMatrix * binormal;
	tangentToEyeMatrix[2] = gl_NormalMatrix * gl_Normal;
}
